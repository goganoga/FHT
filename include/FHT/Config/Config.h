/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 17.01.21
*  Copyright (C) goganoga 2021
***************************************/
#ifndef FHTCONFIG_H
#define FHTCONFIG_H
#include <memory>
#include <string>
#include <fstream>
#include <iterator>

#ifdef DBPOSTGRESQL //use define DBPOSTGRESQL
#include "FHT/Interface/DBFacade/postgresConfiguration.h"
#endif
#include "FHT/LoggerStream.h"

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

namespace FHT {
    void skeleton_daemon();
    struct FHTDefaultConfig {
        std::string host_server = "localhost";
        int port_server = 10800;
        int worker_server = 0;

#ifdef DBPOSTGRESQL //use define DBPOSTGRESQL
        FHT::iDBFacade::Configuration db_config;
#endif
        struct log {
            int time = 0;
            int level = 4;
            std::string fileName = "FHT.log";
        };
        log log;
        virtual void SetCostumConfig(boost::property_tree::ptree& root) {}
    };

    template<typename T = FHT::FHTDefaultConfig>
    const T GetFHTConfig(std::string conf_path = "config.json") {
        T config;
        std::ifstream data(conf_path);
        std::stringstream raw(std::string((std::istreambuf_iterator<char>(data)), std::istreambuf_iterator<char>()));
        if (raw.str().size() < 1) {
            FHT::LoggerStream::Log(FHT::LoggerStream::ERR) << METHOD_NAME << "Framework Handler Task core daemon runing.";
            return {};
        }
        boost::property_tree::ptree root;
        boost::property_tree::read_json(raw, root);

        bool daemon_mode = root.get<bool>("daemon", false);
        if (daemon_mode) {
            FHT::skeleton_daemon();
        }

        config.host_server = root.get<std::string>("host_server", "0.0.0.0");
        config.port_server = root.get<int>("port_server", 10800);
        config.worker_server = root.get<int>("worker_server", 0);

#ifdef DBPOSTGRESQL //use define DBPOSTGRESQL
        config.db_config.m_port = root.get<int>("postgres.port", 5432);
        config.db_config.m_worker = root.get<int>("postgres.worker", 10);
        config.db_config.m_host = root.get<std::string>("postgres.host", "");
        config.db_config.m_name = root.get<std::string>("postgres.name", "");
        config.db_config.m_user = root.get<std::string>("postgres.user", "");
        config.db_config.m_pass = root.get<std::string>("postgres.pass", "");
#endif

        config.log.time = root.get<int>("logging.time", 0);
        config.log.fileName = root.get<std::string>("logging.name", "FHT.log");
        config.log.level = root.get<int>("logging.level", 4);

        config.SetCostumConfig(root);
        return config;
    }
}
#endif //FHTCONFIG_H
