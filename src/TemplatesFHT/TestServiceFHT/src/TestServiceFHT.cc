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
#include "iController.h"
#include "iTest.h"
void skeleton_daemon(char* args);
struct inizialaizer {
	FHT::iTest *Test = FHT::iTest::Run.get();
	FHT::iClient *Client = FHT::iConrtoller::webClient.get(); //pre init
	FHT::iServer *Serv = FHT::iConrtoller::webServer.get(); //end
};
int main(int argc, char* argv[])
{
	if (argc > 1) {
		skeleton_daemon(argv[1]);
	} else
		std::cout << "For start daimon - run programm with [daemon] " << std::endl;;
	try {
		inizialaizer inizialaizerServer;
		inizialaizerServer.Serv->setHost("0.0.0.0");
		inizialaizerServer.Serv->setPort(10800);
		inizialaizerServer.Serv->run();
	} catch (std::exception const &e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}
	std::cout << "Press Enter fot quit." << std::endl;
	std::cin.get();
	return 0;
}

#ifdef __linux__ 
void skeleton_daemon(char* args)
{
	if (std::string(args) != std::string("daemon")) {
		std::cout << "Not exist!!!!" << std::endl;
		std::cout << "For start daimon - run programm with [daemon] " << std::endl;
		return;
	}
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
void skeleton_daemon(char* args) {}
#endif