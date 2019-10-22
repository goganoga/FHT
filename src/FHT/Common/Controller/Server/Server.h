/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 06.08.19
*  Copyright (C) goganoga 2019
***************************************/
#ifndef FHTSERVER_H
#define FHTSERVER_H
#include "InitialSer.h"
#include "../../../Interfice/iServer.h"
#include <iostream>
namespace FHT {
	class Server : public iServer {
		static void OnRequest (evhttp_request *req, void *);
		static bool lessen_all;
		std::shared_ptr<InitSer> initSer;
		std::string host = "localhost";
		std::uint16_t port = 10800;
	public:
		Server();
		void setPort(std::uint16_t port_) override final;
		void setHost(std::string host_) override final;
		void run() override final;
		std::string lessenAll(bool flag) override final;
		virtual ~Server() override {};
	};
}
#endif //FHTSERVER_H
