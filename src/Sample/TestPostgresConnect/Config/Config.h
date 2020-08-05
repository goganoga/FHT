/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 05.08.20
*  Copyright (C) goganoga 2019
***************************************/
#ifndef FHTCONFIG_H
#define FHTCONFIG_H
#include <memory>
#include <string>

//use define DBPOSTGRESQL in CMake
#ifdef DBPOSTGRESQL
#include "DBFacade/postgresConfiguration.h"
#endif

namespace Config {
    struct FTHConfig {
        bool daemon_mode = false;
        std::string host_server = "localhost";
        int port_server = 10800;

        FHT::iDBFacade::Configuration db_config;

        struct log {
            int time = 0;
            int level = 4;
            std::string fileName = "FHT.log";
        };
        log log;
        static const FTHConfig get_config(std::string);
    };
    void skeleton_daemon();
}
#endif //FHTCONFIG_H
