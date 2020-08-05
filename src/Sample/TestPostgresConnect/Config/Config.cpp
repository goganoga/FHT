/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 05.08.20
*  Copyright (C) goganoga 2019
***************************************/
#include "Config.h"
#include "LoggerStream.h"

#ifdef __linux__ 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#endif

#include <fstream>
#include <iterator>

#include <json/json.h>

namespace Config {
	const FTHConfig FTHConfig::get_config(std::string conf_path) {
        FTHConfig config;
        Json::Reader reader;
        std::ifstream data(conf_path);
        std::string raw((std::istreambuf_iterator<char>(data)), std::istreambuf_iterator<char>());
        Json::Value root;
        reader.parse(raw, root);
        if (auto daemon = root["daemon"]; !daemon.empty()) {
            config.daemon_mode = daemon.asBool();
        }
        if (auto host_server = root["host_server"]; !host_server.empty()) {
            config.host_server = host_server.asString();
        }
        if (auto port_server = root["port_server"]; !port_server.empty()) {
            config.port_server = port_server.asInt();
        }

        const auto pg = root["postgres"];

        if (auto port = pg["port"]; !port.empty()) {
            config.db_config.m_port = port.asInt();
        }
        if (auto host = pg["host"]; !host.empty()) {
            config.db_config.m_host = host.asString();
        }
        if (auto name = pg["name"]; !name.empty()) {
            config.db_config.m_name = name.asString();
        }
        if (auto user = pg["user"]; !user.empty()) {
            config.db_config.m_user = user.asString();
        }
        if (auto pass = pg["pass"]; !pass.empty()) {
            config.db_config.m_pass = pass.asString();
        }
        if (auto worker = pg["worker"]; !worker.empty()) {
            config.db_config.m_worker = worker.asInt();
        }

        auto log = root["logging"];

        if (auto time = log["time"]; !time.empty()) {
            config.log.time = time.asInt();
        }
        if (auto name = log["name"]; !name.empty()) {
            config.log.fileName = name.asString();
        }
        if (auto level = log["level"]; !level.empty()) {
            config.log.level = level.asInt();
        }

        return config;
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
        openlog("FHT", LOG_PID, LOG_DAEMON);
        syslog(LOG_NOTICE, "Framework Handler Task core daemon runing.");
        FHT::LoggerStream::Log(FHT::LoggerStream::ERR) << METHOD_NAME << "Framework Handler Task core daemon runing.";

    }
#else
    void skeleton_daemon() {}
#endif
}
