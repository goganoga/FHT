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
        struct httpConnect : public iHttpConnect, public std::enable_shared_from_this<httpConnect<S>> {
            template<typename I, typename ...ARGS>
            explicit httpConnect(I& ioc, ARGS& ...ctx) : m_resolver(ioc), m_stream(ioc, ctx...) {}

            virtual ~httpConnect() {}
            
            void run(params paramParse) {
                m_callback = paramParse.m_callback;
                if (!checkSSL(paramParse.host.c_str())) {
                    return;
                }
                m_req.version(11);
                m_req.method(paramParse.m_request.type);
                m_req.target(paramParse.uri);

                if (paramParse.m_request.type == http::verb::post) {
                    m_req.body() = paramParse.m_request.body;
                }
                m_req.set(http::field::host, paramParse.host);
                for (auto a : paramParse.m_request.headers) {
                    m_req.set(a.first, a.second);
                }
                m_req.prepare_payload();

                m_resolver.async_resolve(paramParse.host, paramParse.port, beast::bind_front_handler(&httpConnect::on_resolve, shared_from_this()));
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

            void on_resolve(beast::error_code ec, tcp::resolver::results_type results) {
                if (ec) return fail(ec, "resolve");
                beast::get_lowest_layer(m_stream).expires_after(m_timeout);
                beast::get_lowest_layer(m_stream).async_connect(results, beast::bind_front_handler(&httpConnect::on_connect, shared_from_this()));
            }

            void on_handshake(beast::error_code);

            void on_connect(beast::error_code, tcp::resolver::results_type::endpoint_type);

            void on_write(beast::error_code ec, std::size_t bytes_transferred) {
                boost::ignore_unused(bytes_transferred);
                if (ec) return fail(ec, "write");
                http::async_read(m_stream, m_buffer, m_res, beast::bind_front_handler(&httpConnect::on_read, shared_from_this()));
            }

            void on_read(beast::error_code, std::size_t);

            void on_shutdown(beast::error_code ec) {
                if (ec == net::error::eof) ec = {};
                if (ec) return fail(ec, "shutdown");
            }

            S m_stream;
            tcp::resolver m_resolver;
            beast::flat_buffer m_buffer;
            http::request<http::dynamic_body> m_req;
            http::response<http::dynamic_body> m_res;
            std::function<void(iClient::httpClient::httpResponse)> m_callback;
            std::chrono::seconds m_timeout = std::chrono::seconds(3);
        };

        template<>
        void httpConnect<beast::ssl_stream<beast::tcp_stream>>::on_read(beast::error_code ec, std::size_t bytes_transferred) {
            boost::ignore_unused(bytes_transferred);
            if (ec) return fail(ec, "read");

            iClient::httpClient::httpResponse resp;
            for (auto& h : m_res.base()) {
                resp.headers.emplace(h.name_string(), h.value());
            }
            resp.body = m_res.body();

            resp.status = m_res.result_int();
            m_callback(resp);

            beast::get_lowest_layer(m_stream).expires_after(m_timeout);
            m_stream.async_shutdown(beast::bind_front_handler(&httpConnect::on_shutdown, shared_from_this()));
        }

        template<typename S>
        void httpConnect<S>::on_read(beast::error_code ec, std::size_t bytes_transferred) {
            boost::ignore_unused(bytes_transferred);
            if (ec) return fail(ec, "read");

            iClient::httpClient::httpResponse resp;
            for (auto& h : m_res.base()) {
                resp.headers.emplace(h.name_string(), h.value());
            }
            resp.body = m_res.body();

            resp.status = m_res.result_int();
            m_callback(resp);

            m_stream.socket().shutdown(tcp::socket::shutdown_both, ec);
            on_shutdown(ec);
        }

        template<>
        bool httpConnect<beast::ssl_stream<beast::tcp_stream>>::checkSSL(char const* host) {
            if (!SSL_set_tlsext_host_name(m_stream.native_handle(), host)) {
                beast::error_code ec{ static_cast<int>(::ERR_get_error()), net::error::get_ssl_category() };
                std::cerr << ec.message() << "\n";
                return false;
            }
            return true;
        }

        template<typename S>
        bool httpConnect<S>::checkSSL(char const* host) {
            return true;
        }

        template<>
        void httpConnect<beast::ssl_stream<beast::tcp_stream>>::on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type) {
            if (ec) return fail(ec, "connect");
            m_stream.async_handshake(ssl::stream_base::client, beast::bind_front_handler(&httpConnect::on_handshake, shared_from_this()));
        }

        template<>
        void httpConnect<beast::ssl_stream<beast::tcp_stream>>::on_handshake(beast::error_code ec) {
            if (ec) return fail(ec, "handshake");
            beast::get_lowest_layer(m_stream).expires_after(m_timeout);
            http::async_write(m_stream, m_req, beast::bind_front_handler(&httpConnect::on_write, shared_from_this()));
        }
        template<typename S>
        void httpConnect<S>::on_handshake(beast::error_code) {}

        template<typename S>
        void httpConnect<S>::on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type) {
            if (ec) return fail(ec, "connect");
            beast::get_lowest_layer(m_stream).expires_after(m_timeout);
            http::async_write(m_stream, m_req, beast::bind_front_handler(&httpConnect::on_write, shared_from_this()));
        }
    }

    webClient::webClient(iClient::httpClient& request, std::function<void(iClient::httpClient::httpResponse)> func, net::io_context& ioc) : m_params(request, std::move(func)) {
        if (m_params.shema == "https") {
            ssl::context ctx{ ssl::context::tls_client };
            std::shared_ptr<httpConnect<beast::ssl_stream<beast::tcp_stream>>> session(new httpConnect<beast::ssl_stream<beast::tcp_stream>>(net::make_strand(ioc), m_ctx));
            session->run(m_params);
            m_http_connect = session;
        }
        else {
            std::shared_ptr<httpConnect<beast::tcp_stream>> session(new httpConnect<beast::tcp_stream>(net::make_strand(ioc)));
            session->run(m_params);
            m_http_connect = session;
        }
    }
}
