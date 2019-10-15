/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 14.10.19
*  Copyright (C) goganoga 2019
***************************************/
#include "Client.h"
#include "../Controller.h"
#include "../../../Interfice/iController.h"
#define HOSTNAME_MAX_SIZE 255

#if defined(__APPLE__) && defined(__clang__)
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

#include <stdio.h>
//#include <assert.h>
#include <stdlib.h>
#include <string.h>
//#include <errno.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <wincrypt.h>

#define snprintf _snprintf
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
#include <future>
namespace {
	struct webClient {
		webClient(std::string url, std::string body, std::function<void(FHT::iClient::respClient)>* func);
		~webClient();
	private:
		static void http_request_done(struct evhttp_request* req, void* ctx);
		static std::function<void(FHT::iClient::respClient)> func_;
		struct event_base* base = nullptr;
		struct evhttp_uri* http_uri = nullptr;
		const char* data_file = nullptr;
		const char* crt = nullptr;
		const char* scheme = nullptr, * host = nullptr, * path = nullptr, * query = nullptr;
		char uri[256] = {};
		int port = 0;
		int retries = 3;
		int timeout = 10;
		SSL_CTX* ssl_ctx = nullptr;
		SSL* ssl = nullptr;
		struct bufferevent* bev = nullptr;
		struct evhttp_connection* evcon = nullptr;
		struct evhttp_request* req = nullptr;
		struct evkeyvalq* output_headers = nullptr;
		struct evbuffer* output_buffer = nullptr;
		enum { HTTP, HTTPS } type = HTTP;
		enum HostnameValidationResult {
			MatchFound,
			MatchNotFound,
			NoSANPresent,
			MalformedCertificate,
			Error
		};
		static HostnameValidationResult matches_subject_alternative_name(const char* hostname, const X509* server_cert);
		static HostnameValidationResult matches_common_name(const char* hostname, const X509* server_cert);
		static HostnameValidationResult validate_hostname(const char* hostname, const X509* server_cert);
#ifdef _WIN32
		int add_cert_for_store(X509_STORE* store, const char* name);
#endif
		static int cert_verify_callback(X509_STORE_CTX* x509_ctx, void* arg);
	};
	std::function<void(FHT::iClient::respClient)> webClient::func_;
	void webClient::http_request_done(struct evhttp_request* req, void* ctx) {
			FHT::iClient::respClient resp;
			char buffer[256];
			int nread;
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
			}
			else {
				auto* InBuf = evhttp_request_get_input_buffer(req);
				auto LenBuf = evbuffer_get_length(InBuf);
				std::unique_ptr<char> postBody(new char[LenBuf + 1]);
				postBody.get()[LenBuf] = 0;
				evbuffer_copyout(InBuf, postBody.get(), LenBuf);
				resp.body = postBody.get();
				resp.status = evhttp_request_get_response_code(req);
			}
			func_(resp);
		}
	webClient::webClient(std::string url, std::string body, std::function<void(FHT::iClient::respClient)>* func) {
			func_ = *func;
			FHT::iClient::respClient resp;
			resp.status = 404;
			http_uri = evhttp_uri_parse(url.c_str());
			if (http_uri == nullptr) {
				resp.body = "Error: Malformed url";
				func_(resp);
				return;
			}
			scheme = evhttp_uri_get_scheme(http_uri);
			if (scheme == nullptr || (strcasecmp(scheme, "https") != 0 && strcasecmp(scheme, "http") != 0)) {
				resp.body = "Error: Url must be http or https";
				func_(resp);
				return;
			}
			host = evhttp_uri_get_host(http_uri);
			if (host == nullptr) {
				resp.body = "Error: Url must have a host";
				func_(resp);
				return;
			}

			port = evhttp_uri_get_port(http_uri);
			if (port == -1)
				port = (strcasecmp(scheme, "http") == 0) ? 80 : 443;
			path = evhttp_uri_get_path(http_uri);
			if (strlen(path) == 0)
				path = "/";
			query = evhttp_uri_get_query(http_uri);
			if (query == nullptr)
				snprintf(uri, sizeof(uri) - 1, "%s", path);
			else
				snprintf(uri, sizeof(uri) - 1, "%s?%s", path, query);
			uri[sizeof(uri) - 1] = '\0';
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
				func_(resp);
				return;
			}

			ssl_ctx = SSL_CTX_new(SSLv23_method());
			if (!ssl_ctx) {
				resp.body = "Error: Openssl SSL_CTX_new failed";
				func_(resp);
				return;
			}
			if (crt == nullptr) {
				X509_STORE* store;
				store = SSL_CTX_get_cert_store(ssl_ctx);
#ifdef _WIN32
				if (add_cert_for_store(store, "CA") < 0 ||
					add_cert_for_store(store, "AuthRoot") < 0 ||
					add_cert_for_store(store, "ROOT") < 0) {
					resp.body = "Error: Openssl X509_STORE_set_default_paths failed";
					func_(resp);
					return;
				}
#else // _WIN32
				if (X509_STORE_set_default_paths(store) != 1) {
					resp.body = "Error: Openssl X509_STORE_set_default_paths failed";
					func_(resp);
					return;
				}
#endif // _WIN32
			} else {
				if (SSL_CTX_load_verify_locations(ssl_ctx, crt, nullptr) != 1) {
					resp.body = "Error: Openssl SSL_CTX_load_verify_locations failed";
					func_(resp);
					return;
				}
			}
			SSL_CTX_set_verify(ssl_ctx, SSL_VERIFY_PEER, nullptr);
			SSL_CTX_set_cert_verify_callback(ssl_ctx, cert_verify_callback, (void*)host);
			base = event_base_new();
			if (!base) {
				resp.body = "Error: New connection failed";
				func_(resp);
				return;
			}
			ssl = SSL_new(ssl_ctx);
			if (ssl == nullptr) {
				resp.body = "Error: Create OpenSSL session failed";
				func_(resp);
				return;
			}
#ifdef SSL_CTRL_SET_TLSEXT_HOSTNAME
			SSL_set_tlsext_host_name(ssl, host);
#endif
			if (strcasecmp(scheme, "http") == 0)
				bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);
			else {
				type = HTTPS;
				bev = bufferevent_openssl_socket_new(base, -1, ssl, BUFFEREVENT_SSL_CONNECTING, BEV_OPT_CLOSE_ON_FREE | BEV_OPT_DEFER_CALLBACKS);
			}
			if (bev == nullptr) {
				resp.body = "Error: Can't read buffer with openssl";
				func_(resp);
				return;
			}
			bufferevent_openssl_set_allow_dirty_shutdown(bev, 1);
			evcon = evhttp_connection_base_bufferevent_new(base, nullptr, bev, host, port);
			if (evcon == nullptr) {
				resp.body = "Error: Can't read buffer with";
				func_(resp);
				return;
			}
			if (retries > 0)
				evhttp_connection_set_retries(evcon, retries);
			if (timeout >= 0)
				evhttp_connection_set_timeout(evcon, timeout);
			req = evhttp_request_new(&http_request_done, bev);
			if (req == nullptr) {
				resp.body = "Error: Not create request";
				func_(resp);
				return;
			}
			output_headers = evhttp_request_get_output_headers(req);
			evhttp_add_header(output_headers, "Host", host);
			evhttp_add_header(output_headers, "Connection", "close");
			if (!body.empty()) {
				char buf[1024];
				output_buffer = evhttp_request_get_output_buffer(req);
				evbuffer_add(output_buffer, body.c_str(), body.length());
				evutil_snprintf(body.data(), sizeof(body.data()) - 1, "%lu", (unsigned long)body.length());
				evhttp_add_header(output_headers, "Content-Length", buf);
			}
			if (evhttp_make_request(evcon, req, !body.empty() ? EVHTTP_REQ_POST : EVHTTP_REQ_GET, uri)) {
				resp.body = "Error: Can't make request";
				func_(resp);
				return;
			}
			event_base_dispatch(base);
		}
	webClient::~webClient() {
		if (evcon)
			evhttp_connection_free(evcon);
		if (http_uri)
			evhttp_uri_free(http_uri);
		if (base)
			event_base_free(base);

		if (ssl_ctx)
			SSL_CTX_free(ssl_ctx);
		if (type == HTTP && ssl)
			SSL_free(ssl);
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

		delete data_file;
		delete crt;
		delete scheme, host, path, query;
		delete bev;
		delete req;
		delete output_headers;
		delete output_buffer;
	}
	webClient::HostnameValidationResult webClient::matches_subject_alternative_name(const char* hostname, const X509* server_cert) {
			HostnameValidationResult result = MatchNotFound;
			int i;
			int san_names_nb = -1;
			STACK_OF(GENERAL_NAME)* san_names = nullptr;
			san_names = (STACK_OF(GENERAL_NAME)*)X509_get_ext_d2i((X509*)server_cert, NID_subject_alt_name, NULL, nullptr);
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
	webClient::HostnameValidationResult webClient::matches_common_name(const char* hostname, const X509* server_cert) {
			int common_name_loc = -1;
			X509_NAME_ENTRY* common_name_entry = nullptr;
			ASN1_STRING* common_name_asn1 = nullptr;
			const char* common_name_str = nullptr;
			common_name_loc = X509_NAME_get_index_by_NID(X509_get_subject_name((X509*)server_cert), NID_commonName, -1);
			if (common_name_loc < 0)
				return Error;
			common_name_entry = X509_NAME_get_entry(X509_get_subject_name((X509*)server_cert), common_name_loc);
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
	webClient::HostnameValidationResult webClient::validate_hostname(const char* hostname, const X509* server_cert) {
			HostnameValidationResult result;
			if ((hostname == nullptr) || (server_cert == nullptr))
				return Error;
			result = matches_subject_alternative_name(hostname, server_cert);
			if (result == NoSANPresent)
				result = matches_common_name(hostname, server_cert);
			return result;
	}
#ifdef _WIN32
	int webClient::add_cert_for_store(X509_STORE* store, const char* name) {
		HCERTSTORE sys_store = nullptr;
		PCCERT_CONTEXT ctx = nullptr;
		int resp = 0;
		sys_store = CertOpenSystemStore(0, name);
		if (!sys_store)
			return -1;
		while ((ctx = CertEnumCertificatesInStore(sys_store, ctx))) {
			X509* x509 = d2i_X509(NULL, (unsigned char const**)& ctx->pbCertEncoded,
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
	int webClient::cert_verify_callback(X509_STORE_CTX* x509_ctx, void* arg) {
			char cert_str[256];
			const char* host = (const char*)arg;
			const char* res_str = "X509_verify_cert failed";
			HostnameValidationResult res = Error;
			int ok_so_far = 0;
			X509* server_cert = nullptr;
			ok_so_far = X509_verify_cert(x509_ctx);
			server_cert = X509_STORE_CTX_get_current_cert(x509_ctx);
			if (ok_so_far) {
				res = validate_hostname(host, server_cert);
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
			X509_NAME_oneline(X509_get_subject_name(server_cert), cert_str, sizeof(cert_str));
			if (res == MatchFound)
				return 1;
			else
				return 0;
		}
}
namespace FHT {
	std::shared_ptr<iClient> Conrtoller::getClient() { 
		auto static a = std::make_shared<Client>();
		return a;
	}
	std::string Client::post(std::string url, std::string body){
		std::promise<std::string> pr;
		std::future<std::string> barrier_future = pr.get_future();
		std::function<void(FHT::iClient::respClient)> func([&pr](FHT::iClient::respClient a) {
			pr.set_value(a.body);
			});
		webClient a(url, body, &func);
		barrier_future.wait();
		return barrier_future.get();
	}
	std::string Client::get(std::string url){
		if (url.empty() || url.length() < 6 || (url.substr(0, 6) != "ftp://" && url.substr(0, 7) != "http://" && url.substr(0, 8) != "https://"))
			return "No correct url";
		std::promise<std::string> pr;
		std::future<std::string> barrier_future = pr.get_future();
		std::function<void(FHT::iClient::respClient)> func([&pr](FHT::iClient::respClient a) {
			pr.set_value(a.body);
		});
		webClient a(url, std::string(), &func);
		barrier_future.wait();
		return barrier_future.get();
	}
	void Client::postAsync(std::string url, std::string body, std::function<void(respClient)> func){
		webClient a(url, body, &func);
	}
	void Client::getAsync(std::string url, std::function<void(respClient)> func){
		webClient a(url, std::string(), &func);
	}

}