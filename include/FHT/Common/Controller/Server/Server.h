/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 06.08.19
*  Copyright (C) goganoga 2019
***************************************/
#ifndef FHTSERVER_H
#define FHTSERVER_H
#include "FHT/Interface/Common/iServer.h"

#include <iostream>
#include <map>
#include <thread>
#include <memory>
#include <string>

#ifdef WITH_COROUTINE
#include <boost/asio/spawn.hpp>
#else 
#include <boost/asio/strand.hpp>
#include <boost/asio/dispatch.hpp>
#endif

namespace FHT {
    struct ParseUri {
        std::string location;
        std::map<std::string, std::string> uriParam;
    };
    class Server : public iServer {
        std::shared_ptr<boost::asio::io_context> m_ioc;
        std::string m_host = "localhost";
        int m_port = 10800;
        int m_worker = std::thread::hardware_concurrency();
        bool volatile m_is_run = true;
        std::vector<std::thread> m_pool;
    public:
        Server();
        void setWorker(int worker) override final;
        void setPort(int port) override final;
        void setHost(std::string host) override final;
        void run() override final;
        std::string lessenAll(bool flag) override final;
        virtual ~Server() override;
        static bool m_lessen_all;
    };
}
#endif //FHTSERVER_H
