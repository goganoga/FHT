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
        FHT::LoggerStream::Log(FHT::LoggerStream::INFO) << METHOD_NAME << what;
    }

    struct HttpServer : public std::enable_shared_from_this<HttpServer> {
        HttpServer() {
            lessen_all = Server::m_lessen_all;
        }
        ParseUri parseUrl(std::string uri) const {
            std::string request(uri);
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
        struct send_lambda_coro {
            beast::tcp_stream& stream_;
            bool& close_;
            beast::error_code& ec_;
            net::yield_context yield_;
            send_lambda_coro(beast::tcp_stream& stream, bool& close, beast::error_code& ec, net::yield_context yield) : stream_(stream), close_(close), ec_(ec), yield_(yield) {}
            template<bool isRequest, class Body, class Fields>
            void operator()(http::message<isRequest, Body, Fields>&& msg) const {
                close_ = msg.need_eof();
                http::serializer<isRequest, Body, Fields> sr{ msg };
                http::async_write(stream_, sr, yield_[ec_]);
            }
        };
        http::response<http::dynamic_body> bad_request(http::request<http::dynamic_body> req, std::string why) const {
            http::response<http::dynamic_body> res{ http::status::bad_request, req.version() };
            res.set(http::field::server, "FHT Server");
            res.set(http::field::content_type, "text/html");
            res.keep_alive(req.keep_alive());
            auto& body = res.body();
            FHT::LoggerStream::Log(FHT::LoggerStream::ERR) << METHOD_NAME << "Http: 400" << req.target() << why;
            body.commit(boost::asio::buffer_copy(body.prepare(why.size()), boost::asio::buffer(why)));
            res.prepare_payload();
            return res;
        }  
        http::response<http::dynamic_body> bad_method(http::request<http::dynamic_body> req, std::string why) const {
            http::response<http::dynamic_body> res{ http::status::method_not_allowed, req.version() };
            res.set(http::field::server, "FHT Server");
            res.set(http::field::content_type, "text/html");
            res.keep_alive(req.keep_alive());
            auto& body = res.body();
            FHT::LoggerStream::Log(FHT::LoggerStream::ERR) << METHOD_NAME << "Http: 405" << req.target() << why;
            body.commit(boost::asio::buffer_copy(body.prepare(why.size()), boost::asio::buffer(why)));
            res.prepare_payload();
            return res;
        };
        http::response<http::dynamic_body> not_found(http::request<http::dynamic_body> req, std::string why) const {
            http::response<http::dynamic_body> res{ http::status::not_found, req.version() };
            res.set(http::field::server, "FHT Server");
            res.set(http::field::content_type, "text/html");
            res.keep_alive(req.keep_alive());
            why = "The resource '" + std::string(req.target()) + "' was not found.";
            FHT::LoggerStream::Log(FHT::LoggerStream::INFO) << METHOD_NAME << "Http: 404" << req.target() << why;
            auto& body = res.body();
            body.commit(boost::asio::buffer_copy(body.prepare(why.size()), boost::asio::buffer(why)));
            res.prepare_payload();
            return res;
        };
        http::response<http::dynamic_body> server_error(http::request<http::dynamic_body> req, std::string why) const {
            http::response<http::dynamic_body> res{ http::status::internal_server_error, req.version() };
            res.set(http::field::server, "FHT Server");
            res.set(http::field::content_type, "text/html");
            res.keep_alive(req.keep_alive());
            why = "An error occurred: '" + std::string(why) + "'";
            auto& body = res.body();
            FHT::LoggerStream::Log(FHT::LoggerStream::FATAL) << METHOD_NAME << "Http: 500" << req.target() << why;
            body.commit(boost::asio::buffer_copy(body.prepare(why.size()), boost::asio::buffer(why)));
            res.prepare_payload();
            return res;
        };

        void session(beast::tcp_stream& stream, net::yield_context yield) const {
            bool close = false;
            beast::error_code ec;
            beast::flat_buffer buffer;
            send_lambda_coro lambda{ stream, close, ec, yield };
            for (;;) {
                stream.expires_after(std::chrono::seconds(30));
                http::request<http::dynamic_body> req;
                http::async_read(stream, buffer, req, yield[ec]);
                if (ec == http::error::end_of_stream) break;
                if (ec) return fail(ec, "read");
                if (req.method() != http::verb::get && req.method() != http::verb::post && req.method() != http::verb::put && req.method() != http::verb::head)
                    return lambda(bad_request(req, "Unknown HTTP method"));
                if (req.method() == http::verb::head) {
                    http::response<http::empty_body> res{ http::status::ok, req.version() };
                    res.set(http::field::server, "FHT Server");
                    res.keep_alive(req.keep_alive());
                    return lambda(std::move(res));
                }
                ParseUri uri = parseUrl(std::string(req.target()));
                FHT::iHendler::dataRequest dataReq;
                std::shared_ptr<FHT::iHendler::uniqueHendler> func;
                dataReq.params = uri.uriParam;
                for (auto& h : req.base()) {
                    dataReq.headers.emplace(h.name_string(), h.value());
                }
                auto info = stream.socket().remote_endpoint();
                dataReq.uri = std::string(req.target());
                dataReq.nextLocation = ".";
                dataReq.ipClient = info.address().to_string();
                dataReq.portClient = static_cast<int>(info.port());
                dataReq.body = req.body();
                std::string loc = lessen_all ? "head" : uri.location;
                loc.append("/");
                bool isWS = websocket::is_upgrade(req);
                for (size_t i = loc.size() - 1; i > 0; i--) {
                    if (loc.at(i) == '/' || loc.at(i - 1) == '/') {
                        if (isWS) {
                            func = H->getUniqueHendler(FHT::webSocket(loc.substr(0, i)));
                        }
                        else {
                            func = H->getUniqueHendler(loc.substr(0, i));
                        }
                        if (func) {
                            dataReq.nextLocation += loc.substr(i, loc.size() - (i + 1));
                            break;
                        }
                    }
                }
                if (!func) return lambda(not_found(req, "Unknown HTTP handler"));
                try {
                    if (isWS) {
                        auto ws = std::make_shared<websocket::stream<beast::tcp_stream>>(std::move(stream.socket()));
                        auto wsSub = std::make_shared<wsSubscriber>();
                        ws->set_option(websocket::stream_base::timeout::suggested(beast::role_type::server));
                        ws->set_option(websocket::stream_base::decorator(
                            [](websocket::response_type& res) {
                                res.set(http::field::server, "FHT Server");
                            }));
                        ws->async_accept(req, yield);
                        if (ec) fail(ec, "accept");
                        FHT::LoggerStream::Log(FHT::LoggerStream::DEBUG) << METHOD_NAME << "WebSocket: OK" << req.target();
                        wsSub->close = [ws, wsSub]() mutable {
                            if (ws) {
                                ws->close(websocket::close_code::normal);
                                ws.reset();
                            }
                        };
                        wsSub->publisher = [ws](std::string& str) {
                            beast::flat_buffer buffer;
                            buffer.commit(boost::asio::buffer_copy(buffer.prepare(str.size()), boost::asio::buffer(str)));
                            if (!ws) return false;
                            ws->text(true);
                            ws->write(buffer.data());
                            return true;
                        };

                        dataReq.WSInstanse = std::weak_ptr<wsSubscriber>(wsSub);
                        FHT::iHendler::dataResponse result = (*func)(dataReq);
                        for (;;) {
                            beast::flat_buffer buffer;
                            ws->async_read(buffer, yield[ec]);
                            if (ec || ec == websocket::error::closed) {
                                if (wsSub && wsSub->deleter) {
                                    wsSub->deleter();
                                }
                                wsSub.reset();
                                if (ws) ws.reset();
                                if (ec) return fail(ec, "read");
                                break;
                            } 
                            if (wsSub && wsSub->subscriber) {
                                if (ws->got_text()) {
                                    wsSub->subscriber(boost::beast::buffers_to_string(buffer.data()));
                                }
                            }
                        }
                        return;
                    }  
                    FHT::iHendler::dataResponse send = (*func)(dataReq);

                    if (send.filePath.empty()) {
                        http::response<http::dynamic_body> res{ http::status::ok, req.version() };
                        res.keep_alive(req.keep_alive());
                        res.set(http::field::server, "FHT Server");
                        for (auto a : send.headers) {
                            res.set(a.first, a.second);
                        }
                        res.body() = send.body;
                        res.prepare_payload();
                        FHT::LoggerStream::Log(FHT::LoggerStream::DEBUG) << METHOD_NAME << req.target() << "Http: OK";// << boost::beast::buffers_to_string(send.body.data());
                        lambda(std::move(res));
                    }
                    else {
                        http::response_parser<http::file_body> res;
                        res.get().keep_alive(req.keep_alive());
                        res.get().set(http::field::server, "FHT Server");
                        for (auto a : send.headers) {
                            res.get().set(a.first, a.second);
                        }
                        res.body_limit(1024 * 1024 * 512);
                        res.get().body().open(send.filePath.c_str(), beast::file_mode::scan, ec);
                        FHT::LoggerStream::Log(FHT::LoggerStream::DEBUG) << METHOD_NAME << req.target() << "Http: OK" << "send file";
                        lambda(std::move(res.get()));
                    }
                }
                catch (const std::string e) {
                    return lambda(bad_method(req, e));
                }
                catch (const char* e) {
                    return lambda(bad_request(req, e));
                }
                catch (...) {
                    return lambda(server_error(req, "Fatal"));
                }
                if (ec)
                    return fail(ec, "write");
                if (close) {
                    // This means we should close the connection, usually because
                    // the response indicated the "Connection: close" semantic.
                    break;
                }
            }
            stream.socket().shutdown(tcp::socket::shutdown_send, ec);
        }
        void listener(net::io_context& ioc, tcp::endpoint endpoint, net::yield_context yield) const {
            beast::error_code ec;
    
            tcp::acceptor acceptor(ioc);
            acceptor.open(endpoint.protocol(), ec);
            if (ec) return fail(ec, "open");
    
            acceptor.set_option(net::socket_base::reuse_address(true), ec);
            if (ec) return fail(ec, "set_option");
    
            acceptor.bind(endpoint, ec);
            if (ec) return fail(ec, "bind");
    
            acceptor.listen(net::socket_base::max_listen_connections, ec);
            if (ec) return fail(ec, "listen");
    
            for (;;) {
                tcp::socket socket(ioc);
                acceptor.async_accept(socket, yield[ec]);
                if (ec) fail(ec, "accept");
                else
                    boost::asio::spawn(
                        acceptor.get_executor(),
                        std::bind(
                            &HttpServer::session,
                            shared_from_this(),
                            beast::tcp_stream(std::move(socket)),
                            std::placeholders::_1));
            }
        }
    private:
        bool lessen_all;
        std::shared_ptr<iHendler> H = Conrtoller::getHendler();
        std::shared_ptr<iTask> T = Conrtoller::getTask();
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
            if(m_ioc) m_ioc->stop();
            m_ioc.reset(new boost::asio::io_context{ m_worker });
            boost::asio::io_context& d = *m_ioc;
            boost::asio::spawn(*m_ioc,
                std::bind(&HttpServer::listener,std::make_shared<HttpServer>(),
                    std::ref(*m_ioc), tcp::endpoint{ net::ip::make_address(m_host), static_cast<unsigned short>(m_port) }, std::placeholders::_1));

            m_pool.reserve(m_worker);
            for (auto i = m_worker; i > 0; --i) {
                m_pool.emplace_back([&]{
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
        catch (std::exception const &e) {
            FHT::LoggerStream::Log(FHT::LoggerStream::ERR) << METHOD_NAME <<  "Error create instanse server: " << e.what();
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
