/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 06.08.19
*  Copyright (C) goganoga 2019
***************************************/
#ifndef FHTCOREFHT_H
#define FHTCOREFHT_H
#include "FHT/Common/Controller/Controller.h"
#include "FHT/Interface/DBFacade/iDBFacade.h"

namespace FHT {
    struct FHTDefaultConfig;
}

namespace CoreFHT {
    template<typename C = FHT::FHTDefaultConfig>
    void InitCore(const C config) {
        try {
            auto Logger = FHT::Conrtoller::getLogger();
            Logger->setTimeDump(config.log.time);
            Logger->setLevelVerboseLogging(static_cast<FHT::iLogger::Verbose>(config.log.level));
            if (!config.log.fileName.empty()) {
                Logger->setFileLogging(config.log.fileName);
            }
#ifdef DBPOSTGRESQL //use define DBPOSTGRESQL
            if (!config.db_config.m_host.empty() && !config.db_config.m_name.empty() &&
                !config.db_config.m_user.empty() && !config.db_config.m_pass.empty()) {
                auto DBConnect = FHT::iDBFacade::DBConnector<FHT::iDBFacade::iDBSettings>();
                DBConnect->setConfiguration(config.db_config);
                DBConnect->run();
                FHT::LoggerStream::Log(FHT::LoggerStream::INFO) << METHOD_NAME << "DB connect" << config.db_config.m_host << config.db_config.m_port;
            }
#endif
            if (config.worker_server > 0) {
                auto Serv = FHT::Conrtoller::getServer();
                Serv->setHost(config.host_server);
                Serv->setPort(config.port_server);
                Serv->setWorker(config.worker_server);
                Serv->run();
            }

            FHT::LoggerStream::Log(FHT::LoggerStream::INFO) << METHOD_NAME << "Start Server";
            FHT::LoggerStream::Log(FHT::LoggerStream::INFO) << METHOD_NAME << "Server host" << config.host_server;
            FHT::LoggerStream::Log(FHT::LoggerStream::INFO) << METHOD_NAME << "Server port" << config.port_server;
        }
        catch (std::exception const& e) {
            FHT::LoggerStream::Log(FHT::LoggerStream::FATAL) << METHOD_NAME << e.what();
        }
    }
    
    inline auto TaskManager = FHT::Conrtoller::getTask();
    inline auto HendlerManager = FHT::Conrtoller::getHendler();
    inline auto QueryDB = FHT::iDBFacade::DBConnector<FHT::iDBFacade::iDBConnect>();
}
#endif // FHTCOREFHT_H
