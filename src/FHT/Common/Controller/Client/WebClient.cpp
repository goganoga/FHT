/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 14.10.19
*  Copyright (C) goganoga 2019
***************************************/
#include "WebClient.h"
#define HOSTNAME_MAX_SIZE 255
#if defined(__APPLE__) && defined(__clang__)
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif
#ifdef _WIN32
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <wincrypt.h>
#define strcasecmp _stricmp
#else
#include <sys/socket.h>
#include <netinet/in.h>
#endif
#include <event2/bufferevent_ssl.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/http.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/x509v3.h>
#include "HostCheckCurl.h"
#include "LoggerStream.h"
#include <mutex>

namespace FHT{
    namespace {
        void parceHttpRequestParam(evhttp_request* req, std::map<std::string, std::string>& http_request_param) {
            struct evkeyvalq* request_input = evhttp_request_get_input_headers(req);
            for (struct evkeyval* tqh_first = request_input->tqh_first; &tqh_first->next != nullptr; ) {
                http_request_param.emplace(tqh_first->key, tqh_first->value);
                tqh_first = tqh_first->next.tqe_next;
            }
        }
        std::map<int, std::function<void(iClient::httpClient::httpResponse)>> map_binder_done;
    }

    void webClient::httpRequestDone(evhttp_request* req, void* ctx) {
        iClient::httpClient::httpResponse resp;
        char buffer[256];
        if (!req || !evhttp_request_get_response_code(req)) {
            struct bufferevent* bev = (struct bufferevent*) ctx;
            unsigned long oslerr;
            int printed_err = 0;
            int errcode = EVUTIL_SOCKET_ERROR();
            while ((oslerr = bufferevent_get_openssl_error(bev))) {
                ERR_error_string_n(oslerr, buffer, sizeof(buffer));
                printed_err = 1;
            }
            resp.body = "Error: Some request failed";
            resp.status = 404;
            FHT::LoggerStream::Log(FHT::LoggerStream::WARN) << METHOD_NAME << resp.body;
        }
        else {
            auto* InBuf = evhttp_request_get_input_buffer(req);
            auto LenBuf = evbuffer_get_length(InBuf);
            std::unique_ptr<char> postBody(new char[LenBuf + 1]);
            postBody.get()[LenBuf] = 0;
            evbuffer_copyout(InBuf, postBody.get(), LenBuf);
            resp.body = postBody.get();
            resp.status = evhttp_request_get_response_code(req);
            parceHttpRequestParam(req, resp.headers);
        }
        auto func = map_binder_done[reinterpret_cast<int>(req)];
        if (func) {
            func(resp);
        }
    }

    webClient::webClient(iClient::httpClient& request, std::function<void(iClient::httpClient::httpResponse)>* func) {
        m_callback = *func;
        std::string& url = request.url;
        std::string& body = request.body;
        iClient::httpClient::httpResponse resp;
        resp.status = 404;
        std::unique_ptr<evhttp_uri, decltype(&evhttp_uri_free)> http_uri(evhttp_uri_parse(url.c_str()), &evhttp_uri_free);
        if (!http_uri) {
            resp.body = "Error: Malformed url";
            FHT::LoggerStream::Log(FHT::LoggerStream::ERR) << METHOD_NAME << resp.body;
            (*func)(resp);
            return;
        }
        const char* scheme = evhttp_uri_get_scheme(http_uri.get());
        if (!scheme || (strcasecmp(scheme, "https") != 0 && strcasecmp(scheme, "http") != 0)) {
            resp.body = "Error: Url must be http or https";
            FHT::LoggerStream::Log(FHT::LoggerStream::ERR) << METHOD_NAME << resp.body;
            (*func)(resp);
            return;
        }
        const char* host = evhttp_uri_get_host(http_uri.get());
        if (host == nullptr) {
            resp.body = "Error: Url must have a host";
            FHT::LoggerStream::Log(FHT::LoggerStream::ERR) << METHOD_NAME << resp.body;
            (*func)(resp);
            return;
        }
        int port = evhttp_uri_get_port(http_uri.get());
        if (port == -1)
            port = (strcasecmp(scheme, "http") == 0) ? 80 : 443;
        const char* path = evhttp_uri_get_path(http_uri.get());
        if (strlen(path) == 0)
            path = "/";
        const char* query = evhttp_uri_get_query(http_uri.get());
        std::string uri;
        if (query == nullptr)
            uri += path;
        else{
            uri += path;
            uri += "?";
            uri += query;}
#if (OPENSSL_VERSION_NUMBER < 0x10100000L) || \
    (defined(LIBRESSL_VERSION_NUMBER) && LIBRESSL_VERSION_NUMBER < 0x20700000L)
        // Initialize OpenSSL
        SSL_library_init();
        ERR_load_crypto_strings();
        SSL_load_error_strings();
        OpenSSL_add_all_algorithms();
#endif
        if (!RAND_poll()) {
            resp.body = "Error: Openssl RAND_poll failed";
            FHT::LoggerStream::Log(FHT::LoggerStream::ERR) << METHOD_NAME << resp.body;
            (*func)(resp);
            return;
         }
         std::unique_ptr<SSL_CTX, decltype (&SSL_CTX_free)> ssl_ctx(SSL_CTX_new(SSLv23_method()) ,&SSL_CTX_free);
        if (!ssl_ctx) {
            resp.body = "Error: Openssl SSL_CTX_new failed";
            FHT::LoggerStream::Log(FHT::LoggerStream::ERR) << METHOD_NAME << resp.body;
            (*func)(resp);
            return;
         }
         X509_STORE* store;
         store = SSL_CTX_get_cert_store(ssl_ctx.get());
#ifdef _WIN32
         if (addCertForStore(store, "CA") < 0 ||
             addCertForStore(store, "AuthRoot") < 0 ||
             addCertForStore(store, "ROOT") < 0) {
             resp.body = "Error: Openssl X509_STORE_set_default_paths failed";
             FHT::LoggerStream::Log(FHT::LoggerStream::ERR) << METHOD_NAME << resp.body;
             (*func)(resp);
             return;
         }
#else // _WIN32
         if (X509_STORE_set_default_paths(store) != 1) {
             resp.body = "Error: Openssl X509_STORE_set_default_paths failed";
             FHT::LoggerStream::Log(FHT::LoggerStream::ERR) << METHOD_NAME << resp.body;
             (*func)(resp);
             return;
         }
#endif // _WIN32
         SSL_CTX_set_verify(ssl_ctx.get(), SSL_VERIFY_PEER, nullptr);
         SSL_CTX_set_cert_verify_callback(ssl_ctx.get(), certVerifyCallback, (void*)host);

         const std::unique_ptr<event_base, decltype(&event_base_free)> base(event_base_new(), &event_base_free);
         if (!base) {
            resp.body = "Error: New connection failed";
            FHT::LoggerStream::Log(FHT::LoggerStream::ERR) << METHOD_NAME << resp.body;
            (*func)(resp);
            return;
         }
         std::unique_ptr<SSL, decltype (&SSL_free)> ssl(SSL_new(ssl_ctx.get()) ,&SSL_free);
         if (ssl == nullptr) {
            resp.body = "Error: Create OpenSSL session failed";
            FHT::LoggerStream::Log(FHT::LoggerStream::ERR) << METHOD_NAME << resp.body;
            (*func)(resp);
            return;
         }
#ifdef SSL_CTRL_SET_TLSEXT_HOSTNAME
        SSL_set_tlsext_host_name(ssl.get(), host);
#endif
        struct bufferevent* bev = nullptr;
        if (strcasecmp(scheme, "http") == 0)
            bev = bufferevent_socket_new(base.get(), -1, BEV_OPT_CLOSE_ON_FREE);
        else {
            bev = bufferevent_openssl_socket_new(base.get(), -1, ssl.get(), BUFFEREVENT_SSL_CONNECTING, BEV_OPT_CLOSE_ON_FREE | BEV_OPT_DEFER_CALLBACKS);
        }
        if (bev == nullptr) {
            resp.body = "Error: Can't read buffer with openssl";
            FHT::LoggerStream::Log(FHT::LoggerStream::ERR) << METHOD_NAME << resp.body;
            (*func)(resp);
            return;
        }
        bufferevent_openssl_set_allow_dirty_shutdown(bev, 1);
        std::unique_ptr<evhttp_connection, decltype(&evhttp_connection_free)> evcon(evhttp_connection_base_bufferevent_new(base.get(), nullptr, bev, host, port) ,&evhttp_connection_free);
        if (evcon == nullptr) {
            resp.body = "Error: Can't read buffer with";
            FHT::LoggerStream::Log(FHT::LoggerStream::ERR) << METHOD_NAME << resp.body;
            (*func)(resp);
            return;
        }
        evhttp_connection_set_retries(evcon.get(), 3);
        evhttp_connection_set_timeout(evcon.get(), 10);

        struct evhttp_request* req = evhttp_request_new(&httpRequestDone, bev);

        if (req == nullptr) {
            resp.body = "Error: Not create request";
            FHT::LoggerStream::Log(FHT::LoggerStream::ERR) << METHOD_NAME << resp.body;
            (*func)(resp);
            return;
        }
        int id = reinterpret_cast<int>(req);
        map_binder_done.emplace(id, [&](iClient::httpClient::httpResponse r) {
            m_callback(r);
        });

        struct evkeyvalq* output_headers = evhttp_request_get_output_headers(req);
        evhttp_add_header(output_headers, "Host", host);
        evhttp_add_header(output_headers, "Connection", "close");
        for (auto& header : request.headers) {
            evhttp_add_header(output_headers, header.first.c_str(), header.second.c_str());
        }
        if (request.type == iClient::httpClient::Type::POST) {
            struct evbuffer* output_buffer;
            if (request.body_is_file_name) {
                FILE* f = fopen(request.body.c_str(), "rb");
                char buf[1024];
                size_t s;
                size_t bytes = 0;

                if (!f) {
                    resp.body = "Error: File not read";
                    FHT::LoggerStream::Log(FHT::LoggerStream::ERR) << METHOD_NAME << resp.body << request.body;
                    (*func)(resp);
                    return;
                }

                output_buffer = evhttp_request_get_output_buffer(req);
                while ((s = fread(buf, 1, sizeof(buf), f)) > 0) {
                    evbuffer_add(output_buffer, buf, s);
                    bytes += s;
                }
                evutil_snprintf(buf, sizeof(buf) - 1, "%lu", (unsigned long)bytes);
                evhttp_add_header(output_headers, "Content-Length", buf);
                fclose(f);
            }
            else {
                output_buffer = evhttp_request_get_output_buffer(req);

                std::shared_ptr<char> buf(new char[body.size() + 1]);
                buf.get()[body.size()] = 0;
                memcpy(buf.get(), body.data(), body.size());

                evbuffer_add(output_buffer, buf.get(), body.size());
                evhttp_add_header(output_headers, "Content-Length", std::to_string(body.size()).c_str());
            }
        }
        if (evhttp_make_request(evcon.get(), req, request.type == iClient::httpClient::Type::POST ? EVHTTP_REQ_POST : EVHTTP_REQ_GET, uri.c_str())) {
            resp.body = "Error: Can't make request";
            FHT::LoggerStream::Log(FHT::LoggerStream::ERR) << METHOD_NAME << resp.body;
            (*func)(resp);
            return;
        }
        event_base_dispatch(base.get());
        evhttp_request_free(req); 
        map_binder_done.erase(id);
        delete this;
    }
    webClient::~webClient() {
#if (OPENSSL_VERSION_NUMBER < 0x10100000L) || \
    (defined(LIBRESSL_VERSION_NUMBER) && LIBRESSL_VERSION_NUMBER < 0x20700000L)
        EVP_cleanup();
        ERR_free_strings();
#if OPENSSL_VERSION_NUMBER < 0x10000000L
        ERR_remove_state(0);
#else
        ERR_remove_thread_state(NULL);
#endif
        CRYPTO_cleanup_all_ex_data();
        sk_SSL_COMP_free(SSL_COMP_get_compression_methods());
#endif
    }
    webClient::hostnameValidation webClient::matchesSubjectAlternativeName(const char* hostname, const X509* serverCert) {
            hostnameValidation result = MatchNotFound;
            int i;
            int san_names_nb = -1;
            STACK_OF(GENERAL_NAME)* san_names = nullptr;
            san_names = (STACK_OF(GENERAL_NAME)*)X509_get_ext_d2i((X509*)serverCert, NID_subject_alt_name, nullptr, nullptr);
            if (san_names == nullptr)
                return NoSANPresent;
            san_names_nb = sk_GENERAL_NAME_num(san_names);
            for (i = 0; i < san_names_nb; i++) {
                const GENERAL_NAME* current_name = sk_GENERAL_NAME_value(san_names, i);
                if (current_name->type == GEN_DNS) {
                    const char* dns_name = (char*)ASN1_STRING_get0_data(current_name->d.dNSName);
                    if ((size_t)ASN1_STRING_length(current_name->d.dNSName) != strlen(dns_name)) {
                        result = MalformedCertificate;
                        break;
                    }
                    else {
                        if (Curl_cert_hostcheck(dns_name, hostname) == CURL_HOST_MATCH) {
                            result = MatchFound;
                            break;
                        }
                    }
                }
            }
            sk_GENERAL_NAME_pop_free(san_names, GENERAL_NAME_free);
            return result;
        }
    webClient::hostnameValidation webClient::matchesCommonName(const char* hostname, const X509* serverCert) {
            int common_name_loc = -1;
            X509_NAME_ENTRY* common_name_entry = nullptr;
            ASN1_STRING* common_name_asn1 = nullptr;
            const char* common_name_str = nullptr;
            common_name_loc = X509_NAME_get_index_by_NID(X509_get_subject_name((X509*)serverCert), NID_commonName, -1);
            if (common_name_loc < 0)
                return Error;
            common_name_entry = X509_NAME_get_entry(X509_get_subject_name((X509*)serverCert), common_name_loc);
            if (common_name_entry == nullptr)
                return Error;
            common_name_asn1 = X509_NAME_ENTRY_get_data(common_name_entry);
            if (common_name_asn1 == nullptr)
                return Error;
            common_name_str = (char*)ASN1_STRING_get0_data(common_name_asn1);
            if ((size_t)ASN1_STRING_length(common_name_asn1) != strlen(common_name_str))
                return MalformedCertificate;
            if (Curl_cert_hostcheck(common_name_str, hostname) == CURL_HOST_MATCH)
                return MatchFound;
            else
                return MatchNotFound;
    }
    webClient::hostnameValidation webClient::validateHostname(const char* hostname, const X509* serverCert) {
            hostnameValidation result;
            if ((hostname == nullptr) || (serverCert == nullptr))
                return Error;
            result = matchesSubjectAlternativeName(hostname, serverCert);
            if (result == NoSANPresent)
                result = matchesCommonName(hostname, serverCert);
            return result;
    }
#ifdef _WIN32
    int webClient::addCertForStore(X509_STORE* store, const char* name) {
        HCERTSTORE sys_store = nullptr;
        PCCERT_CONTEXT ctx = nullptr;
        int resp = 0;
        sys_store = CertOpenSystemStore(0, name);
        if (!sys_store)
            return -1;
        while ((ctx = CertEnumCertificatesInStore(sys_store, ctx))) {
            X509* x509 = d2i_X509(nullptr, (unsigned char const**)& ctx->pbCertEncoded,
                ctx->cbCertEncoded);
            if (x509) {
                X509_STORE_add_cert(store, x509);
                X509_free(x509);
            }
            else {
                resp = -1;
                break;
            }
        }
        CertCloseStore(sys_store, 0);
        return resp;
    }
#endif
    int webClient::certVerifyCallback(X509_STORE_CTX* x509Ctx, void* arg) {
            char cert_str[256];
            const char* host = (const char*)arg;
            const char* res_str = "X509_verify_cert failed";
            hostnameValidation res = Error;
            int ok_so_far = 0;
            X509* serverCert = nullptr;
            ok_so_far = X509_verify_cert(x509Ctx);
            serverCert = X509_STORE_CTX_get_current_cert(x509Ctx);
            if (ok_so_far) {
                res = validateHostname(host, serverCert);
                switch (res) {
                case MatchFound:
                    res_str = "MatchFound";
                    break;
                case MatchNotFound:
                    res_str = "MatchNotFound";
                    break;
                case NoSANPresent:
                    res_str = "NoSANPresent";
                    break;
                case MalformedCertificate:
                    res_str = "MalformedCertificate";
                    break;
                case Error:
                    res_str = "Error";
                    break;
                default:
                    res_str = "WTF!";
                    break;
                }
            }
            X509_NAME_oneline(X509_get_subject_name(serverCert), cert_str, sizeof(cert_str));
            if (res == MatchFound)
                return 1;
            else
                return 0;
        }
}
