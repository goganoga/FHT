/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 17.01.2021
*  Copyright (C) goganoga 2021
***************************************/
#include "FHT/Common/Controller/Server/Server.h"
#include "FHT/Common/Controller/Controller.h"
#include "FHT/Common/Template.h"
#include "FHT/LoggerStream.h"

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/spawn.hpp>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

namespace FHT {

    bool Server::m_lessen_all = false;

    void fail(beast::error_code ec, char const* what) {
        FHT::LoggerStream::Log(FHT::LoggerStream::INFO) << METHOD_NAME << ec.message() << what;
    }

    struct HttpServer : public std::enable_shared_from_this<HttpServer> {
        HttpServer(tcp::socket&& socket) : m_stream(std::move(socket)), m_lambda(*this) {
            m_lessen_all = Server::m_lessen_all;
        }
        ParseUri parseUrl() const {
            std::string request(m_req.target());
            std::map<std::string, std::string> get_param;
            if (auto f = request.find("?"); f < request.size()) {
                std::string request_get = request.substr(f + 1);
                request = request.substr(0, f);

                std::string key;
                std::string value;
                for (int i = 0; i <= request_get.length(); i++) {
                    if (request_get[i] == '&' || i == request_get.length()) {
                        get_param.emplace(key, value);
                        key.clear();
                        value.clear();
                    }
                    else if (request_get[i] == '=' && key.empty()) {
                        key.swap(value);
                    }
                    else {
                        value += request_get[i];
                    }
                }
            }
            return { request, get_param };
        }
        http::response<http::dynamic_body> bad_request(std::string why) const {
            http::response<http::dynamic_body> res{ http::status::bad_request, m_req.version() };
            res.set(http::field::server, "FHT Server");
            res.set(http::field::content_type, "text/html");
            res.keep_alive(m_req.keep_alive());
            auto& body = res.body();
            FHT::LoggerStream::Log(FHT::LoggerStream::ERR) << METHOD_NAME << "Http: 400" << m_req.target() << why;
            body.commit(boost::asio::buffer_copy(body.prepare(why.size()), boost::asio::buffer(why)));
            res.prepare_payload();
            return res;
        }
        http::response<http::dynamic_body> bad_method(std::string why) const {
            http::response<http::dynamic_body> res{ http::status::method_not_allowed, m_req.version() };
            res.set(http::field::server, "FHT Server");
            res.set(http::field::content_type, "text/html");
            res.keep_alive(m_req.keep_alive());
            auto& body = res.body();
            FHT::LoggerStream::Log(FHT::LoggerStream::ERR) << METHOD_NAME << "Http: 405" << m_req.target() << why;
            body.commit(boost::asio::buffer_copy(body.prepare(why.size()), boost::asio::buffer(why)));
            res.prepare_payload();
            return res;
        };
        http::response<http::dynamic_body> not_found(std::string why) const {
            http::response<http::dynamic_body> res{ http::status::not_found, m_req.version() };
            res.set(http::field::server, "FHT Server");
            res.set(http::field::content_type, "text/html");
            res.keep_alive(m_req.keep_alive());
            why = "The resource '" + std::string(m_req.target()) + "' was not found.";
            FHT::LoggerStream::Log(FHT::LoggerStream::INFO) << METHOD_NAME << "Http: 404" << m_req.target() << why;
            auto& body = res.body();
            body.commit(boost::asio::buffer_copy(body.prepare(why.size()), boost::asio::buffer(why)));
            res.prepare_payload();
            return res;
        };
        http::response<http::dynamic_body> server_error(std::string why) const {
            http::response<http::dynamic_body> res{ http::status::internal_server_error, m_req.version() };
            res.set(http::field::server, "FHT Server");
            res.set(http::field::content_type, "text/html");
            res.keep_alive(m_req.keep_alive());
            why = "An error occurred: '" + std::string(why) + "'";
            auto& body = res.body();
            FHT::LoggerStream::Log(FHT::LoggerStream::FATAL) << METHOD_NAME << "Http: 500" << m_req.target() << why;
            body.commit(boost::asio::buffer_copy(body.prepare(why.size()), boost::asio::buffer(why)));
            res.prepare_payload();
            return res;
        };
        
        void run() {
            net::dispatch(m_stream.get_executor(), beast::bind_front_handler(&HttpServer::do_read, shared_from_this()));
        }

        void do_read() {
            m_req = {};
            m_stream.expires_after(std::chrono::seconds(30));
            http::async_read(m_stream, m_buffer, m_req, beast::bind_front_handler(&HttpServer::on_read, shared_from_this()));
        }

        void on_read(beast::error_code ec, std::size_t bytes_transferred) {
            boost::ignore_unused(bytes_transferred);
            if (ec == http::error::end_of_stream) return do_close();
            if (ec) return fail(ec, "read");
            if (m_req.method() != http::verb::get && m_req.method() != http::verb::post && m_req.method() != http::verb::put && m_req.method() != http::verb::head)
                return m_lambda(bad_request("Unknown HTTP method"));
            if (m_req.method() == http::verb::head) {
                http::response<http::empty_body> res{ http::status::ok, m_req.version() };
                res.set(http::field::server, "FHT Server");
                res.keep_alive(m_req.keep_alive());
                return m_lambda(std::move(res));
            }
            ParseUri uri = parseUrl();
            m_dataReq.params = uri.uriParam;
            for (auto& h : m_req.base()) {
                m_dataReq.headers.emplace(h.name_string(), h.value());
            }
            auto info = m_stream.socket().remote_endpoint();
            m_dataReq.uri = std::string(m_req.target());
            m_dataReq.nextLocation = ".";
            m_dataReq.ipClient = info.address().to_string();
            m_dataReq.portClient = static_cast<int>(info.port());
            m_dataReq.body = m_req.body();
            std::string loc = m_lessen_all ? "head" : uri.location;
            loc.append("/");
            bool isWS = websocket::is_upgrade(m_req);
            for (size_t i = loc.size() - 1; i > 0; i--) {
                if (loc.at(i) == '/' || loc.at(i - 1) == '/') {
                    if (isWS) {
                        m_func = H->getUniqueHendler(FHT::webSocket(loc.substr(0, i)));
                    }
                    else {
                        m_func = H->getUniqueHendler(loc.substr(0, i));
                    }
                    if (m_func) {
                        m_dataReq.nextLocation += loc.substr(i, loc.size() - (i + 1));
                        break;
                    }
                }
            }
            if (!m_func) return m_lambda(not_found("Unknown HTTP handler"));

            try {
                if (isWS) {
                    run_ws();
                    return;
                }
                FHT::iHendler::dataResponse send = (*m_func)(m_dataReq);
                if (send.filePath.empty()) {
                    http::response<http::dynamic_body> res{ http::status::ok, m_req.version() };
                    res.keep_alive(m_req.keep_alive());
                    res.set(http::field::server, "FHT Server");
                    for (auto a : send.headers) {
                        res.set(a.first, a.second);
                    }
                    res.body() = send.body;
                    res.prepare_payload();
                    FHT::LoggerStream::Log(FHT::LoggerStream::DEBUG) << METHOD_NAME << m_req.target() << "Http: OK";// << boost::beast::buffers_to_string(send.body.data());
                    m_lambda(std::move(res));
                }
                else {
                    http::response_parser<http::file_body> res;
                    res.get().keep_alive(m_req.keep_alive());
                    res.get().set(http::field::server, "FHT Server");
                    for (auto a : send.headers) {
                        res.get().set(a.first, a.second);
                    }
                    res.body_limit(1024 * 1024 * 512);
                    res.get().body().open(send.filePath.c_str(), beast::file_mode::scan, ec);
                    FHT::LoggerStream::Log(FHT::LoggerStream::DEBUG) << METHOD_NAME << m_req.target() << "Http: OK" << "send file";
                    m_lambda(std::move(res.get()));
                }
            }
            catch (const std::string e) {
                return m_lambda(bad_method(e));
            }
            catch (const char* e) {
                return m_lambda(bad_request(e));
            }
            catch (...) {
                return m_lambda(server_error("Fatal"));
            }
        }

        void on_write(bool close, beast::error_code ec, std::size_t bytes_transferred) {
            boost::ignore_unused(bytes_transferred);
            if (ec) return fail(ec, "write");
            if (close) {
                // This means we should close the connection, usually because
                // the response indicated the "Connection: close" semantic.
                return do_close();
            }
            m_res = nullptr;
            do_read();
        }

        void do_close() {
            beast::error_code ec;
            m_stream.socket().shutdown(tcp::socket::shutdown_send, ec);
        }

        void run_ws() {
            m_ws = std::make_shared<websocket::stream<beast::tcp_stream>>(std::move(m_stream.socket()));
            m_ws->set_option(websocket::stream_base::timeout::suggested(beast::role_type::server));
            m_ws->set_option(websocket::stream_base::decorator(
                [](websocket::response_type& res) {
                    res.set(http::field::server, "FHT Server");
                }));
            m_ws->async_accept(m_req, beast::bind_front_handler(&HttpServer::on_accept_ws, shared_from_this()));
        }

        void on_accept_ws(beast::error_code ec) {
            if (ec) return fail(ec, "accept");
            FHT::LoggerStream::Log(FHT::LoggerStream::DEBUG) << METHOD_NAME << "WebSocket: OK" << m_req.target();
            m_wsSub = std::make_shared<wsSubscriber>();
            m_wsSub->close = [&]() mutable {
                if (m_ws) {
                    m_ws->close(websocket::close_code::normal);
                    m_ws.reset();
                }
            };
            m_wsSub->publisher = [&](std::string& str) {
                beast::flat_buffer buffer;
                buffer.commit(boost::asio::buffer_copy(buffer.prepare(str.size()), boost::asio::buffer(str)));
                if (!m_ws) return false;
                m_ws->text(true);
                m_ws->write(buffer.data());
                return true;
            };

            m_dataReq.WSInstanse = std::weak_ptr<wsSubscriber>(m_wsSub);
            (*m_func)(m_dataReq);
            do_read_ws();
        }

        void do_read_ws() {
            m_ws->async_read(m_buffer, beast::bind_front_handler(&HttpServer::on_read_ws, shared_from_this()));
        }

        void on_read_ws(beast::error_code ec, std::size_t bytes_transferred) {
            boost::ignore_unused(bytes_transferred);
            if (ec || ec == websocket::error::closed) {
                if (m_wsSub && m_wsSub->deleter) {
                    m_wsSub->deleter();
                }
                m_wsSub.reset();
                if (m_ws) m_ws.reset();
                if (ec) return fail(ec, "read");
                return;
            }
            if (m_wsSub && m_wsSub->subscriber) {
                if (m_ws->got_text()) {
                    const net::mutable_buffer buf = m_buffer.data();
                    std::string buf = boost::beast::buffers_to_string(buf);
                    m_wsSub->subscriber(buf);
                }
            }
            m_ws->async_read(m_buffer, beast::bind_front_handler(&HttpServer::on_read_ws, shared_from_this()));
        }
    private:
        struct send_lambda_async {
            explicit send_lambda_async(HttpServer& self) : m_self(self) {}
            template<bool isRequest, class Body, class Fields>
            void operator()(http::message<isRequest, Body, Fields>&& msg) const {
                auto sp = std::make_shared<http::message<isRequest, Body, Fields>>(std::move(msg));
                m_self.m_res = sp;
                http::async_write(m_self.m_stream, *sp,
                    beast::bind_front_handler(&HttpServer::on_write, m_self.shared_from_this(), sp->need_eof()));
            }
            HttpServer& m_self;
        };
        beast::tcp_stream m_stream;
        beast::flat_buffer m_buffer;

        http::request<http::dynamic_body> m_req;
        FHT::iHendler::dataRequest m_dataReq;

        std::shared_ptr<websocket::stream<beast::tcp_stream>> m_ws;
        std::shared_ptr<FHT::iHendler::uniqueHendler> m_func;
        std::shared_ptr<wsSubscriber> m_wsSub;

        std::shared_ptr<void> m_res;
        send_lambda_async m_lambda;
        bool m_lessen_all;

        std::shared_ptr<iHendler> H = Conrtoller::getHendler();
        std::shared_ptr<iTask> T = Conrtoller::getTask();
    };

    struct HttpListener : public std::enable_shared_from_this<HttpListener> {
        HttpListener(net::io_context& ioc, tcp::endpoint endpoint) : m_ioc(ioc), m_acceptor(net::make_strand(ioc)) {
            beast::error_code ec;
            m_acceptor.open(endpoint.protocol(), ec);
            if (ec) {
                fail(ec, "open");
                return;
            }
            m_acceptor.set_option(net::socket_base::reuse_address(true), ec);
            if (ec) {
                fail(ec, "set_option");
                return;
            }
            m_acceptor.bind(endpoint, ec);
            if (ec) {
                fail(ec, "bind");
                return;
            }
            m_acceptor.listen(net::socket_base::max_listen_connections, ec);
            if (ec) {
                fail(ec, "listen");
                return;
            }
        }
        void do_accept() {
            m_acceptor.async_accept(net::make_strand(m_ioc),
                beast::bind_front_handler(&HttpListener::on_accept, shared_from_this()));
        }
    private:
        void on_accept(beast::error_code ec, tcp::socket socket) {
            if (ec) {
                fail(ec, "accept");
            }
            else {
                std::make_shared<HttpServer>(std::move(socket))->run();
            }
            do_accept();
        }

        net::io_context& m_ioc;
        tcp::acceptor m_acceptor;
    };

    std::shared_ptr<iServer> Conrtoller::getServer() {
        auto static a = std::make_shared<Server>();
        return a;
    }
    Server::Server() {}
    Server::~Server() {
        m_is_run = false;
        m_ioc->stop();
        m_ioc.reset();
    }
    void Server::run() {
        try {
            if (m_ioc) m_ioc->stop();
            m_ioc.reset(new boost::asio::io_context{ m_worker });
            boost::asio::io_context& d = *m_ioc;
            std::make_shared<HttpListener>(*m_ioc, tcp::endpoint{ net::ip::make_address(m_host), static_cast<unsigned short>(m_port) })->do_accept();

            m_pool.reserve(m_worker);
            for (auto i = m_worker; i > 0; --i) {
                m_pool.emplace_back([&] {
#ifdef BLOCKING_IO
                    m_ioc->run();
#else 
                    for (; m_is_run; ) {
                        m_ioc->poll();
                        std::this_thread::sleep_for(std::chrono::milliseconds(50));
                    }
#endif// BLOCKING_IO
                    });
            }
        }
        catch (std::exception const& e) {
            FHT::LoggerStream::Log(FHT::LoggerStream::ERR) << METHOD_NAME << "Error create instanse server: " << e.what();
        }
    }

    std::string Server::lessenAll(bool flag) {
        m_lessen_all = flag;
        return flag ? "head" : "";
    }
    void Server::setWorker(int worker) {
        m_worker = worker;
    }
    void Server::setPort(int port) {
        m_port = port;
    }
    void Server::setHost(std::string host) {
        m_host = host;
    }
}
