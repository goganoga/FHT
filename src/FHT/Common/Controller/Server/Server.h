/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 06.08.19
*  Copyright (C) goganoga 2019
***************************************/
#ifndef FHTSERVER_H
#define FHTSERVER_H
#include "InitialSer.h"
#include "Common/iServer.h"
#include <iostream>
#include <map>
namespace FHT {
    class Server : public iServer {
        static void OnRequest (evhttp_request *req, void *);
        static bool lessen_all_;
        static std::string parseGetUrl(evhttp_request* req, std::map<std::string, std::string>& get_param);
        static std::string parceHttpRequestParam(evhttp_request* req, std::map<std::string, std::string>& http_request_param);
        std::shared_ptr<InitSer> initSer_;
        std::string host_ = "localhost";
        int port_ = 10800;
        int worker_ = std::thread::hardware_concurrency();
    public:
        Server();
        void setWorker(int worker) override final;
        void setPort(int port) override final;
        void setHost(std::string host) override final;
        void run() override final;
        std::string lessenAll(bool flag) override final;
        virtual ~Server() override {};
    };
}
#endif //FHTSERVER_H
