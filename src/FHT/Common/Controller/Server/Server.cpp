/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 06.08.19
*  Copyright (C) goganoga 2019
***************************************/
#include "Server.h"
#include "../Controller.h"
#include "../../../Interfice/iController.h"
#include "../../../lib/websocket/websocket.h"
#include "../../../lib/websocket/connection.h"
#include "../../../lib/websocket/user.h"
#include <evhttp.h>
#include <event2/http.h>
#include <event2/event.h>
#include <event2/bufferevent.h>
vector<user_t*> user_vec;
namespace FHT {
	std::shared_ptr<iServer> Conrtoller::getServer() { 
		auto static a = std::make_shared<Server>();
		return a;
	}
	Server::Server() {
	}
	bool Server::lessen_all = false;
	void Server::run() {
		try {
			initSer.reset(new InitSer(&Server::OnRequest, host, port));
		}
		catch (std::exception const &e)
		{
			std::cerr << "Error create instanse server: " << e.what() << std::endl;
		}
	}
	
	void Server::OnRequest(evhttp_request *req, void *) {
		bool lessen_all_ = lessen_all;
		auto H = FHT::iConrtoller::hendlerManager;
		auto T = FHT::iConrtoller::taskManager;
		auto *OutBuf = evhttp_request_get_output_buffer(req);
		if (evhttp_request_get_command(req) == EVHTTP_REQ_GET || evhttp_request_get_command(req) == EVHTTP_REQ_POST || evhttp_request_get_command(req) == EVHTTP_REQ_PUT) {
			if (!OutBuf) goto err;
			struct evkeyvalq headers;
			auto *InBuf = evhttp_request_get_input_buffer(req);
			auto LenBuf = evbuffer_get_length(InBuf);
			std::unique_ptr<char> postBody(new char[LenBuf + 1]);
			postBody.get()[LenBuf] = 0;
			evbuffer_copyout(InBuf, postBody.get(), LenBuf);
			evhttp_parse_query(evhttp_request_get_uri(req), &headers);
			auto evhttp_request = evhttp_request_get_evhttp_uri(req);
			auto location = evhttp_uri_get_path(evhttp_request);
			std::map<std::string, std::string> map;
			for (struct evkeyval *tqh_first = headers.tqh_first; &tqh_first->next != nullptr; ) {
				map.emplace(tqh_first->key, tqh_first->value);
				tqh_first = tqh_first->next.tqe_next;
			}

			struct evkeyvalq *avv = evhttp_request_get_input_headers(req);
			std::map<std::string, std::string> map2;
			std::string hea_str;
			for (struct evkeyval* tqh_first = avv->tqh_first; &tqh_first->next != nullptr; ) {
				map2.emplace(tqh_first->key, tqh_first->value);
				hea_str.append(tqh_first->key).append(": ").append(tqh_first->value).append("\r\n");
				tqh_first = tqh_first->next.tqe_next;
			}
			if (!location) goto err;

			FHT::iHendler::data data_;
			data_.map0 = map; //headers

			if (auto a = map2.find("Connection"); a != map2.end() && a->second == "Upgrade") {
				user_t* user = user_create();
				user->wscon->bev = evhttp_connection_get_bufferevent(evhttp_request_get_connection(req));
				for (auto a : map2)
					user->wscon->ws_req_str.append(a.first).append(": ").append(a.second).append("\r\n");

				std::function<void(std::string)> func_ptr;
				func_ptr = [&](std::string id) {
					if (!H->getUniqueHendler(id + "write")) {
						auto write = [&](iHendler::data data, user_t* user) {
							std::string str = data.str1;
							frame_buffer_t* fb = frame_buffer_new(1, 1, str.size(), str.data());
							if (send_a_frame(user->wscon, fb) == 0)
								return "1";
							return "0";
						};
						H->addUniqueHendler(id + "write", std::bind(write, std::placeholders::_1, user)); 
					}
					if (!H->getHendler(id + "dalete")) {
						auto dalete = [H, T](std::string id) {
							T->addTaskOneRun(T->MAIN, [H, id]() {
								H->removeUniqueHendler(id + "read");
								H->removeUniqueHendler(id + "write");
								H->removeHendler(id + "dalete");
								}, 100);
						};
						H->addHendler(id + "dalete", std::bind(dalete, id));
					}
					auto user_del_cb = [H](std::string id) {
						auto func = H->getHendler(id + "dalete");
						if(func)
							func();
					};
					auto user_read_cb = [H](std::string id, std::string msg) {
						iHendler::data data;
						data.str0 = id;
						data.str1 = msg;
						auto func = H->getUniqueHendler("readWebSocket");
						if (func) {
							auto result = func(data);
						}
					};
					user->close_bind = std::bind(user_del_cb, id);
					user->read_bind = std::bind(user_read_cb, id, std::placeholders::_1);
					ws_conn_setcb(user->wscon, FRAME_RECV, frame_recv_cb, user);
					ws_conn_setcb(user->wscon, CLOSE, user_disconnect_cb, user);
					};
				data_.obj0 = (void*)&func_ptr;
				auto func = H->getUniqueHendler("creatorWebSocket");
				auto result = func(data_);
				ws_serve_start(user->wscon);
				bufferevent_enable(user->wscon->bev, EV_WRITE);
				request_read_cb(user->wscon->bev, user->wscon);
				return;

			}
			auto func = H->getUniqueHendler(lessen_all_ ? "head" : location);
			if (!func) goto err;
			data_.str0 = evhttp_request_get_uri(req); //uri
			data_.str1 = location ? location : ""; // location
			data_.str2 = postBody.get(); //postBody
			data_.str3 = evhttp_request_get_host(req); //host
			data_.id = evhttp_uri_get_port(evhttp_request); //port

			evhttp_add_header(evhttp_request_get_output_headers(req), "Content-Type", "text/plain; charset=utf-8");
			evbuffer_add_printf(OutBuf, func(data_).c_str());
			evhttp_clear_headers(&headers);
			evhttp_send_reply(req, HTTP_OK, "", OutBuf); // nead realization webSocket
		}
		else {
		err:
			evbuffer_add_printf(OutBuf, "<html><body><center><h1>404</h1></center></body></html>");
			evhttp_send_reply(req, HTTP_NOTFOUND, "", OutBuf);
		}
	}
	
	//return Hendler lessen name
	std::string Server::lessenAll(bool flag) {
		lessen_all = flag;
		return flag ? "head" : "";
	}
	void Server::setPort(std::uint16_t port_) {
		port = port_;
	}
	void Server::setHost(std::string host_) {
		host = host_;
	}
}