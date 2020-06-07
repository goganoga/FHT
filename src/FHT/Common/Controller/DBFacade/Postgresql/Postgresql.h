/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 05.06.20
*  Copyright (C) goganoga 2020
***************************************/
#ifndef FHTPOSTGRESQL_H
#define FHTPOSTGRESQL_H
#include "Common/iDBFacade.h"
#include "PostgresConnection.h"

#include <cstring>
#include <condition_variable>
#include <functional>
#include <vector>
#include <map>
#include <memory>
#include <mutex>
#include <queue>

#include <libpq-fe.h>

#include "LoggerStream.h"

namespace FHT {
    class Postgres {
        using returnQuery = std::map<std::string, std::vector<std::string>>;
        
        std::string m_host;
        std::string m_name;
        std::string m_user;
        std::string m_pass;
        int m_port = 5432;
        int m_poolCount = 10;
        bool volatile m_isRun = false;

        std::mutex m_mux;
        std::condition_variable m_condition;
        using ptrConnection = std::shared_ptr<PostgresConnection>;
        std::queue<ptrConnection> m_pool_conn;


        bool run();
        void setHost(std::string arg);
        void setName(std::string arg);
        void setUser(std::string arg);
        void setPass(std::string arg);
        void setPort(int arg);
        void setWorker(int arg);
    protected:
        ptrConnection getConnection();
        void freeConnection(ptrConnection connection);

    public:
        returnQuery queryPrivate(std::string& query, int size, const char* const* params);

        Postgres() = default;
        virtual ~Postgres();

        friend class dbFacade;
    };
}
#endif //FHTPOSTGRESQL_H
