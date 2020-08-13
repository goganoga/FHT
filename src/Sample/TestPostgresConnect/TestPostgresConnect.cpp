/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 06.08.19
*  Copyright (C) goganoga 2019
***************************************/
#include <fstream>
#include <iostream>

#include "iController.h"
#include "LoggerStream.h"
#include "iDBFacade.h"
#include "Config/Config.h"

using namespace FHT::iDBFacade;

struct inizialaizer {
    std::shared_ptr<iDBSettings> DBConnect = DBConnector<iDBSettings>();
    FHT::iLogger* Logger = FHT::iConrtoller::logger.get();
};

int main(int argc, char* argv[])
{
    std::string conf_path("configSQL.json");
    if (argc > 1) {
        if (argv[1] == "-c" || argv[1] == "--config" || argv[2]) {
            conf_path = std::string(argv[2]);
        }
        else {
            std::cout << "-c | --config <path_to_config>               path to file config.json" << std::endl;
            return 1;
        }
    }
    try {
        auto config = Config::FTHConfig::get_config(conf_path);
        if (config.daemon_mode) {
            Config::skeleton_daemon();
        }

        inizialaizer inizialaizerServer;
        inizialaizerServer.Logger->setTimeDump(config.log.time);
        inizialaizerServer.Logger->setLevelVerboseLogging(static_cast<FHT::iLogger::Verbose>(config.log.level));
        inizialaizerServer.Logger->setFileLogging(config.log.fileName);

        inizialaizerServer.DBConnect->setConfiguration(config.db_config);
        inizialaizerServer.DBConnect->run();

        //emulation work in programm
        auto result = DBConnector<iDBConnect>()->Query("select version()");

        std::cout << "Connection version PostgreSQL: " << result["version"][0] << std::endl;
    }
    catch (std::exception const& e) {
        FHT::LoggerStream::Log(FHT::LoggerStream::FATAL) << METHOD_NAME << e.what();
    }
    return 0;
}