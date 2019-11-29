/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 14.10.19
*  Copyright (C) goganoga 2019
***************************************/
#ifndef FHTICLIENT_H
#define FHTICLIENT_H
#include <memory>
#include <cstdint>
#include <functional>
namespace FHT {
	struct iClient {
		struct respClient {
			int status;
			std::string body;
		};
		virtual std::string post(std::string url, std::string body) = 0;
		virtual std::string get(std::string url) = 0;
		virtual void postAsync(std::string url, std::string body, std::function<void(respClient)> func) = 0;
		virtual void getAsync(std::string url, std::function<void(respClient)> func) = 0;
		virtual ~iClient() = default;
	};
}
#endif //FHTICLIENT_H
