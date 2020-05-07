/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 06.08.19
*  Copyright (C) goganoga 2019
***************************************/
#ifdef __linux__ 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#endif

#include <iostream>
#include <fstream>
#include <iterator>
#include <json/json.h>
#include "iController.h"
#include "iTest.h"
void skeleton_daemon();
struct inizialaizer {
	FHT::iTest *Test = FHT::iTest::Run.get();
	FHT::iClient *Client = FHT::iConrtoller::webClient.get(); //pre init
	FHT::iServer *Serv = FHT::iConrtoller::webServer.get(); //end
};
int main(int argc, char* argv[])
{
	std::string conf_path("config.json");
	if (argc > 1) {
		if (argv[1] == "-c" || argv[1] == "--config" || argv[2]) {
			conf_path = std::string(argv[2]);
		} else {
			std::cout << "-c | --config <path_to_config>               path to file config.json" << std::endl;
			return 1;
		}
	}
	try {
		Json::Reader reader;
		std::ifstream data(conf_path);
		std::string raw((std::istreambuf_iterator<char>(data)), std::istreambuf_iterator<char>());
		Json::Value root;
		reader.parse(raw, root);
		auto daemon = root["daemon"];
		if (auto daemon_ = daemon.asBool(); daemon_) {
			skeleton_daemon();
		}
		auto host_server = root["host_server"];
		auto port_server = root["port_server"];
		std::cout << "Start server host - " << (host_server.empty() ? "0.0.0.0" : host_server.asCString()) << std::endl;
		std::cout << "Start server port - " << (port_server.empty() ? 10800 : port_server.asInt()) << std::endl;
		
		inizialaizer inizialaizerServer;
		inizialaizerServer.Serv->setHost(host_server.empty() ? "0.0.0.0" : host_server.asCString());
		inizialaizerServer.Serv->setPort(port_server.empty() ? 10800 : port_server.asInt());
		inizialaizerServer.Serv->run();
	} catch (std::exception const &e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}
	std::cout << "Press Enter fot quit." << std::endl;
	std::cin.get();
	return 0;
}

#ifdef __linux__ 
void skeleton_daemon()
{
	pid_t pid, sid;
	pid = fork();
	if (pid < 0)
		exit(EXIT_FAILURE);
	if (pid > 0)
		exit(EXIT_SUCCESS);
	umask(0);
	sid = setsid();
	if (sid < 0)
		exit(EXIT_FAILURE);
	if ((chdir("/")) < 0)
		exit(EXIT_FAILURE);
	for (int x = sysconf(_SC_OPEN_MAX); x >= 0; x--)
		close(x);
	openlog("web_service_1", LOG_PID, LOG_DAEMON);
	syslog(LOG_NOTICE, "Web service daemon runing.");

}
#else
void skeleton_daemon() {}
#endif