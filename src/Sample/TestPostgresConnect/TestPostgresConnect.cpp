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

//use define DBPOSTGRESQL in CMake
#ifdef DBPOSTGRESQL
#include "DBFacade/postgresConfiguration.h"
#endif

#include <json/json.h>

using namespace FHT::iDBFacade;

struct inizialaizer {
    FHT::iDBFacade* DBConnect = FHT::iConrtoller::dbFacade.get();
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
            std::cout << "-c | --config <path_to_config>               path to file configSQL.json" << std::endl;
            return 1;
        }
    }
    try {
        WebService::Reader reader;
        std::ifstream data(conf_path);
        std::string raw((std::istreambuf_iterator<char>(data)), std::istreambuf_iterator<char>());
        Json::Value root;
        reader.parse(raw, root);

        auto pg = root["postgres"];
        auto port = pg.empty() ? decltype(pg)() : pg["port"];
        auto host = pg.empty() ? decltype(pg)() : pg["host"];
        auto name = pg.empty() ? decltype(pg)() : pg["name"];
        auto user = pg.empty() ? decltype(pg)() : pg["user"];
        auto pass = pg.empty() ? decltype(pg)() : pg["pass"];
        auto worker = pg.empty() ? decltype(pg)() : pg["worker"];

        auto log = root["logging"];
        auto time = log.empty() ? decltype(log)() : log["time"];
        auto fileName = log.empty() ? decltype(log)() : log["name"];
        auto level = log.empty() ? decltype(log)() : log["level"];

        inizialaizer inizialaizerServer;
        if (!time.empty()) { 
            inizialaizerServer.Logger->setTimeDump(time.asInt());
        }
        if (!fileName.empty()) {
            inizialaizerServer.Logger->setFileLogging(fileName.asCString());
        }
        inizialaizerServer.Logger->setLevelVerboseLogging(level.empty() ? FHT::iLogger::Verbose::INFO : static_cast<FHT::iLogger::Verbose>(level.asInt()));

        Configuration db_config;
        if (!port.empty()) { db_config.m_port = port.asInt(); }
        if (!host.empty()) { db_config.m_host = host.asString(); }
        if (!name.empty()) { db_config.m_name = name.asString(); }
        if (!user.empty()) { db_config.m_user = user.asString(); }
        if (!pass.empty()) { db_config.m_pass = pass.asString(); }
        if (!worker.empty()) { db_config.m_worker = worker.asInt(); }
        inizialaizerServer.DBConnect->setConfiguration(db_config);
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