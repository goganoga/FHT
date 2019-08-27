/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 06.08.19
*  Copyright (C) goganoga 2019
***************************************/
#ifndef FHTISERVER_H
#define FHTISERVER_H
#include <memory>
#include <cstdint>
namespace FHT {
	struct iServer {
		static std::shared_ptr<iServer> Ininstanse;
		virtual void run() = 0;
		virtual void setPort(std::uint16_t port_) = 0;
		virtual void setHost(std::string host_) = 0;
		//return Hendler lessen name
		virtual std::string lessenAll(bool flag) = 0;
		virtual ~iServer() = default;
	};
}
#endif //FHTISERVER_H
