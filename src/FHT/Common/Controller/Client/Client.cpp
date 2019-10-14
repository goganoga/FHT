/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 14.10.19
*  Copyright (C) goganoga 2019
***************************************/
#include "Client.h"
#include "../Controller.h"
#include "../../../Interfice/iController.h"
#include <evhttp.h>
namespace FHT {
	std::shared_ptr<iClient> Conrtoller::getClient() { 
		auto static a = std::make_shared<Client>();
		return a;
	}
	void Client::OnRequest(evhttp_request *req, void *) {

	}
	
	std::string Client::post(std::string url, std::string body){
		return std::string();
	}
	std::string Client::get(std::string url){
		return std::string();
	}
	void Client::postAsync(std::string url, std::string body, std::function<void(void)> func){}
	void Client::getAsync(std::string url, std::function<void(void)> func){}
}