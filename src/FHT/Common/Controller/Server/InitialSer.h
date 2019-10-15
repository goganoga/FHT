/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 06.08.19
*  Copyright (C) goganoga 2019
***************************************/
#ifndef FHTINITIALSER_H
#define FHTINITIALSER_H
#include <stdexcept>
#include <thread>
#include <cstdint>
#include <memory>
#include <vector>
#include <mutex>
#include <functional>

#include <evhttp.h>


class InitSer
{
public:
	InitSer(void(*onRequestHandler_)(evhttp_request *, void *), std::string SrvAddress, std::uint16_t srvPort);
	~InitSer();
private:
	void Start();

private:
	evutil_socket_t Socket = -1;
	using threadPtr = std::unique_ptr<std::thread, std::function<void(std::thread*)>>;
	using threadPool = std::vector<threadPtr>;
	void(*OnRequest)(evhttp_request *, void *);
	bool volatile IsRun = true;
	event_config *cfg = nullptr;
	threadPool threads;
	std::string SrvAddress = "0.0.0.0";
	std::uint16_t SrvPort = 10800;
	std::mutex mu;
};
#endif //FHTINITIALSER_H
