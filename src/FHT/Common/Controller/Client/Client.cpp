/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 14.10.19
*  Copyright (C) goganoga 2019
***************************************/
#include "Client.h"
#include "Controller/Controller.h"
#include "iController.h"
#include "WebClient.h"
#include <future>

namespace FHT {
	std::shared_ptr<iClient> Conrtoller::getClient() { 
		auto static a = std::make_shared<Client>();
		return a;
	}
	Client::Client() {
#ifdef _WIN32
		WORD wVersionRequested = MAKEWORD(2, 2);
		WSADATA wsaData;
		WSAStartup(wVersionRequested, &wsaData);
#endif
	}
	Client::~Client(){
#ifdef _WIN32
		WSACleanup();
#endif
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
		if (url.empty() || url.length() < 6 || (url.substr(0, 7) != "http://" && url.substr(0, 8) != "https://"))
			return "No correct url";
		std::promise<std::string> pr;
		std::future<std::string> barrier_future = pr.get_future();
		std::function<void(FHT::iClient::respClient)> func([&pr](FHT::iClient::respClient a) {
			pr.set_value(a.body);
			//pr.set_value(std::to_string(a.status) + " " + a.body);
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
