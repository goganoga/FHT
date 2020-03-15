/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 06.08.19
*  Copyright (C) goganoga 2019
***************************************/
#include "Server.h"
#include "Controller/Controller.h"
#include "iController.h"
#include "WebSocket/WebSocket.h"
#include "WebSocket/Connection.h"
#include "WebSocket/User.h"
#include <evhttp.h>
#include <event2/http.h>
#include <event2/event.h>
#include <event2/bufferevent.h>

namespace FHT {
    std::shared_ptr<iServer> Conrtoller::getServer() { 
        auto static a = std::make_shared<Server>();
        return a;
    }
    Server::Server() {
    }
    bool Server::lessen_all_ = false;
    void Server::run() {
        try {
            initSer_.reset(new InitSer(&Server::OnRequest, host_, port_));
        }
        catch (std::exception const &e)
        {
            std::cerr << "Error create instanse server: " << e.what() << std::endl;
        }
    }
    
    void Server::OnRequest(evhttp_request *req, void *) {
        bool lessen_all = Server::lessen_all_;
        auto H = FHT::iConrtoller::hendlerManager;
        auto T = FHT::iConrtoller::taskManager;
        auto *OutBuf = evhttp_request_get_output_buffer(req);
        if (evhttp_request_get_command(req) == EVHTTP_REQ_GET || evhttp_request_get_command(req) == EVHTTP_REQ_POST || evhttp_request_get_command(req) == EVHTTP_REQ_PUT) {
            if (!OutBuf) goto err;
            auto* InBuf = evhttp_request_get_input_buffer(req);
            auto LenBuf = evbuffer_get_length(InBuf);
            std::unique_ptr<char> postBody(new char[LenBuf + 1]);
            postBody.get()[LenBuf] = 0;
            evbuffer_copyout(InBuf, postBody.get(), LenBuf);
            auto evhttp_request = evhttp_request_get_evhttp_uri(req);
            const char* location = evhttp_uri_get_path(evhttp_request);

            std::map<std::string, std::string> get_param;
            std::map<std::string, std::string> http_request_param;
            std::string request_get = parseGetUrl(req, get_param);
            std::string http_request_param_str = parceHttpRequestParam(req, http_request_param);
            if (!location) goto err;

            FHT::iHendler::data data_;
            FHT::iHendler::uniqueHendler func;
            data_.map0 = get_param; //get param
            data_.map1 = http_request_param; //get param
            data_.str0 = request_get; //uri
            data_.str1 = "."; //nextLocation
            data_.str3 = evhttp_uri_get_path(evhttp_request); //host
            data_.id = evhttp_uri_get_port(evhttp_request); //port
            std::string loc = lessen_all ? "head" : location;
            loc.append("/");
            auto a = http_request_param.find("Connection");
            auto b = http_request_param.find("Upgrade");
            if (a != http_request_param.end() && a->second == "Upgrade" && b != http_request_param.end() && b->second == "websocket") {
                for (int i = loc.size() - 1; i > 0; i--) {
                    if (loc.at(i) == '/' || loc.at(i - 1) == '/') {
                        func = H->getUniqueHendler(FHT::webSocket(loc.substr(0, i)));
                        if (func) {
                            data_.str1 += loc.substr(i, loc.size() - (i + 1)); //nextLocation
                            break;
                        }
                    }
                }
                if (!func) goto err;
                std::shared_ptr<wsUser> user(new wsUser(evhttp_request_get_connection(req)));
                user->wsConn_->wsReqStr_ = http_request_param_str;
                std::shared_ptr<wsSubscriber> ws(new wsSubscriber());
                ws->publisher = [wsConn = user->wsConn_](std::string& str) {
                    if (!wsConn) return false;
                    std::unique_ptr<wsFrameBuffer> fb(new wsFrameBuffer(1, 1, str.size(), str.data()));
                    return wsConn->writeFrameData(fb.get()) == 200;
                };
                user->readBind_ = [ws](std::string msg) mutable {
                    if (ws && ws->subscriber) {
                        ws->subscriber(msg);
                    }
                };
                user->closeBind_ = [&, ws]() mutable {
                    if (ws && ws->deleter) {
                        ws->deleter();
                    }
                    ws.reset();
                };
                data_.obj1 = std::weak_ptr<wsSubscriber>(ws);
                wsConnect* wsu = user->wsConn_.get();
                wsConnectSetHendler(wsu, wsConnect::FRAME_RECV, [user]() mutable { user->frameRead(); });
                wsConnectSetHendler(wsu, wsConnect::CLOSE, [user]() mutable { user.reset(); });
                auto result = func(data_);
                user->wsConn_->wsServerStart();
                bufferevent_enable(user->wsConn_->bev_, EV_WRITE);
                requestReadHendler(user->wsConn_->bev_, user->wsConn_.get());
                return;

            }
            for (int i = loc.size() - 1; i > 0; i--) {
                if (loc.at(i) == '/' || loc.at(i - 1) == '/') {
                    func = H->getUniqueHendler(loc.substr(0, i));
                    if (func) {
                        data_.str1 += loc.substr( i, loc.size() - (i + 1)); //nextLocation
                        break;
                    }
                }
            }
            data_.str2 = postBody.get(); //postBody
            if (!func) goto err;
            evhttp_add_header(std::move(evhttp_request_get_output_headers(req)), "Content-Type", "*/*; charset=utf-8");
            /*{
                { "txt", "text/plain" },
                { "c", "text/plain" },
                { "h", "text/plain" },
                { "html", "text/html" },
                { "htm", "text/htm" },
                { "css", "text/css" },
                { "gif", "image/gif" },
                { "jpg", "image/jpeg" },
                { "jpeg", "image/jpeg" },
                { "png", "image/png" },
                { "pdf", "application/pdf" },
                { "ps", "application/postscript" },
                { NULL, NULL },
            };
            evhttp_add_header(std::move(evhttp_request_get_output_headers(req)), "Content-Type", "image/*; charset=utf-8")
        };*/
			evbuffer_add_printf(OutBuf, func(data_).c_str());
			evhttp_send_reply(req, HTTP_OK, "", OutBuf);
			
        }
        else {
        err:
            evbuffer_add_printf(OutBuf, "<html><body><center><h1>404</h1></center></body></html>");
            evhttp_send_reply(req, HTTP_NOTFOUND, "", OutBuf);
        }
    }
    
    std::string Server::parseGetUrl(evhttp_request* req, std::map<std::string, std::string>& get_param) {
        std::string request_get(evhttp_request_get_uri(req));
        if (auto f = request_get.find("?"); f < request_get.size()) {
            request_get = request_get.substr(f + 1);
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
        return request_get;
    }
    std::string Server::parceHttpRequestParam(evhttp_request* req, std::map<std::string, std::string>& http_request_param) {
        std::string http_request_param_str;
        struct evkeyvalq* request_input = evhttp_request_get_input_headers(req);
        for (struct evkeyval* tqh_first = request_input->tqh_first; &tqh_first->next != nullptr; ) {
            http_request_param.emplace(tqh_first->key, tqh_first->value);
            http_request_param_str.append(tqh_first->key).append(": ").append(tqh_first->value).append("\r\n");
            tqh_first = tqh_first->next.tqe_next;
        }
        return http_request_param_str;
    }
    //return Hendler lessen name
    std::string Server::lessenAll(bool flag) {
        lessen_all_ = flag;
        return flag ? "head" : "";
    }
    void Server::setPort(std::uint16_t port) {
        port_ = port;
    }
    void Server::setHost(std::string host) {
        host_ = host;
    }
}
