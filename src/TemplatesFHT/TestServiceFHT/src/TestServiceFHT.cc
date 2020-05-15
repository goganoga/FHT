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

#include <chrono>
#include <iostream>
#include <fstream>
#include <iterator>
#include <thread>

#include <json/json.h>
#include "iController.h"
#include "iTest.h"
#include "LoggerStream.h"

void skeleton_daemon();
struct inizialaizer {
    FHT::iTest *Test = FHT::iTest::Run.get();
    FHT::iLogger* Logger = FHT::iConrtoller::logger.get();
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
        auto worker_server = root["worker_server"];
        
        auto log = root["logging"];
        auto time = log.empty() ? decltype(log)() : log["time"];
        auto fileName = log.empty() ? decltype(log)() : log["name"];
        auto level = log.empty() ? decltype(log)() : log["level"];
        
        FHT::iConrtoller::taskManager->setDeltaTime(std::chrono::microseconds(10));
        
        inizialaizer inizialaizerServer;
        
        
        if (!time.empty()) {
            inizialaizerServer.Logger->setTimeDump(time.asInt());
        }
        inizialaizerServer.Logger->setLevelVerboseLogging(level.empty() ? FHT::iLogger::Verbose::INFO : static_cast<FHT::iLogger::Verbose>(level.asInt()));
        if (!fileName.empty()) {
            inizialaizerServer.Logger->setFileLogging(fileName.asCString());
        }
        inizialaizerServer.Serv->setHost(host_server.empty() ? "0.0.0.0" : host_server.asCString());
        inizialaizerServer.Serv->setPort(port_server.empty() ? 10800 : port_server.asInt());
        inizialaizerServer.Serv->setWorker(worker_server.empty() ? 10800 : worker_server.asInt());
        inizialaizerServer.Serv->run();
        
        FHT::LoggerStream::Log(FHT::LoggerStream::INFO) << METHOD_NAME << "Start Server";
        FHT::LoggerStream::Log(FHT::LoggerStream::INFO) << METHOD_NAME << "Server host" << (host_server.empty() ? "0.0.0.0" : host_server.asCString());
        FHT::LoggerStream::Log(FHT::LoggerStream::INFO) << METHOD_NAME << "Server port" << (port_server.empty() ? 10800 : port_server.asInt());
    } catch (std::exception const &e) {
        FHT::LoggerStream::Log(FHT::LoggerStream::ERR) << METHOD_NAME << e.what();
    }
    std::cout << "For exit press Ctrl + C" << std::endl;
    for(;;) std::this_thread::sleep_for(std::chrono::seconds(2));
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