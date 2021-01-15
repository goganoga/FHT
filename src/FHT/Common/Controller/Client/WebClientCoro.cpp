/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 17.01.21
*  Copyright (C) goganoga 2021
***************************************/
#include "FHT/Common/Controller/Client/WebClient.h"
#include "FHT/LoggerStream.h"

#include <mutex>
#include <iostream>

namespace FHT {
    params::params(const iClient::httpClient& request, std::function<void(iClient::httpClient::httpResponse)> callback) :m_request(request), m_callback(callback) {
        std::string str = request.url;
        size_t pos = str.find("://");
        shema = str.substr(0, pos);
        str = str.substr(pos + 3);
        pos = str.find("/");
        uri = str.substr(pos);
        str = str.substr(0, pos);
        pos = str.find(":");
        host = str.substr(0, pos);
        if (pos == std::string::npos) {
            port = shema == "http" ? "80" : "443";
        }
        else {
            port = str.substr(pos + 1);
        }
    }
    
    namespace {
        template<typename S>
        struct httpConnect : public iHttpConnect {
            template<typename I, typename ...ARGS>
            httpConnect(I& ioc, ARGS& ...args) : m_resolver(ioc), m_stream(ioc, args...) {}

            virtual ~httpConnect() {}

            void run(params paramParse, net::yield_context yield) {
                m_callback = paramParse.m_callback;
                if (!checkSSL(paramParse.host.c_str())) {
                    return;
                }
                auto const results = m_resolver.async_resolve(paramParse.host, paramParse.port, yield[m_ec]);
                if (m_ec) return fail(m_ec, "resolve");
                beast::get_lowest_layer(m_stream).expires_after(m_timeout);
                get_lowest_layer(m_stream).async_connect(results, yield[m_ec]);
                if (m_ec) return fail(m_ec, "connm_ect");
                beast::get_lowest_layer(m_stream).expires_after(m_timeout);
                async_handshake(yield[m_ec]);
                if (m_ec) return fail(m_ec, "handshake");
                http::request<http::dynamic_body> req{ paramParse.m_request.type, paramParse.uri, 11 };
                if (paramParse.m_request.type == http::verb::post) {
                    req.body() = paramParse.m_request.body;
                }
                req.set(http::field::host, paramParse.host);
                for (auto a : paramParse.m_request.headers) {
                    req.set(a.first, a.second);
                }
                req.prepare_payload();
                beast::get_lowest_layer(m_stream).expires_after(m_timeout);
                http::async_write(m_stream, req, yield[m_ec]);
                if (m_ec) return fail(m_ec, "write");
                beast::flat_buffer b;
                http::response<http::dynamic_body> res;
                http::async_read(m_stream, b, res, yield[m_ec]);
                if (m_ec) return fail(m_ec, "read");

                iClient::httpClient::httpResponse resp;
                for (auto& h : res.base()) {
                    resp.headers.emplace(h.name_string(), h.value());
                }
                resp.body = res.body();

                resp.status = res.result_int();
                m_callback(resp);

                async_shutdown(yield[m_ec]);
                if (m_ec) return fail(m_ec, "shutdown");
            }
        private:
            void fail(beast::error_code ec, char const* what) {
                iClient::httpClient::httpResponse resp;
                resp.status = 404;
                resp.err.append(what).append(": ").append(ec.message());
                FHT::LoggerStream::Log(FHT::LoggerStream::ERR) << METHOD_NAME << resp.err;
                m_callback(resp);
            }

            bool checkSSL(char const* host);
            void async_handshake(net::yield_context yield);
            void async_shutdown(net::yield_context yield);

            std::function<void(iClient::httpClient::httpResponse)> m_callback;
            beast::error_code m_ec;
            tcp::resolver m_resolver;
            S m_stream;
            std::chrono::seconds m_timeout = std::chrono::seconds(3);

        };

        template<typename S>
        void httpConnect<S>::async_shutdown(net::yield_context yield) {
            m_stream.socket().shutdown(tcp::socket::shutdown_both, m_ec);
        }

        template<>
        void httpConnect<beast::ssl_stream<beast::tcp_stream>>::async_shutdown(net::yield_context yield) {
            beast::get_lowest_layer(m_stream).expires_after(m_timeout);
            m_stream.async_shutdown(yield[m_ec]);
            if (m_ec == net::error::eof) m_ec = {};
        }

        template<typename S>
        void httpConnect<S>::async_handshake(net::yield_context yield) {
        }

        template<>
        void httpConnect<beast::ssl_stream<beast::tcp_stream>>::async_handshake(net::yield_context yield) {
            m_stream.async_handshake(ssl::stream_base::client, yield[m_ec]);
        }

        template<typename S>
        bool httpConnect<S>::checkSSL(char const* host) {
            return true;
        }

        template<>
        bool httpConnect<beast::ssl_stream<beast::tcp_stream>>::checkSSL(char const* host) {
            if (!SSL_set_tlsext_host_name(m_stream.native_handle(), host)) {
                m_ec.assign(static_cast<int>(::ERR_get_error()), net::error::get_ssl_category());
                fail(m_ec, "error SSL");
                return false;
            }
            return true;
        }
    }

    webClient::webClient(iClient::httpClient& request, std::function<void(iClient::httpClient::httpResponse)> func, net::io_context& ioc) : m_params(request, std::move(func)){
        if (m_params.shema == "https") {
            ssl::context ctx{ ssl::context::tls_client };
            std::shared_ptr<httpConnect<beast::ssl_stream<beast::tcp_stream>>> session(new httpConnect<beast::ssl_stream<beast::tcp_stream>>(ioc, m_ctx));
            boost::asio::spawn(ioc, std::bind(
                &httpConnect<beast::ssl_stream<beast::tcp_stream>>::run, session,
                m_params, std::placeholders::_1));
            m_http_connect = session;
        }
        else {
            std::shared_ptr<httpConnect<beast::tcp_stream>> session(new httpConnect<beast::tcp_stream>(ioc));
            boost::asio::spawn(ioc, std::bind(
                &httpConnect<beast::tcp_stream>::run, session,
                m_params, std::placeholders::_1));
            m_http_connect = session;
        }
    }
}
