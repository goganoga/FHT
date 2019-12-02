/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 14.10.19
*  Copyright (C) goganoga 2019
***************************************/
#ifndef FHTCLIENT_H
#define FHTCLIENT_H

#include "Controller/Server/InitialSer.h"
#include "iClient.h"
#include <iostream>
#include <event2/http.h>
namespace FHT {
	class Client : public iClient {
		const std::unique_ptr<event_base, decltype(&event_base_free)> base_;
	public:
		Client();
		std::string post(std::string url, std::string body) override final;
		std::string get(std::string url) override final;
		void postAsync(std::string url, std::string body, std::function<void(respClient)> func) override final;
		void getAsync(std::string url, std::function<void(respClient)> func) override final;
		virtual ~Client() override;
	};
}
#endif //FHTCLIENT_H
