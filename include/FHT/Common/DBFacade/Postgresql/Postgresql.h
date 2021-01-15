/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 05.06.20
*  Copyright (C) goganoga 2020
***************************************/
#ifndef FHTPOSTGRESQL_H
#define FHTPOSTGRESQL_H
#include "FHT/Common/DBFacade/Postgresql/PostgresConnection.h"
#include "FHT/Interface/DBFacade/iDBFacade.h"
#include "FHT/Interface/DBFacade/postgresConfiguration.h"
#include "FHT/LoggerStream.h"

#include <cstring>
#include <condition_variable>
#include <functional>
#include <vector>
#include <map>
#include <memory>
#include <mutex>
#include <queue>

#include <libpq-fe.h>


namespace FHT {
    class Postgres {
        using returnQuery = std::map<std::string, std::vector<std::string>>;
        bool volatile m_isRun = false;

        std::mutex m_mux;
        std::condition_variable m_condition;
        using ptrConnection = std::shared_ptr<PostgresConnection>;
        std::queue<ptrConnection> m_pool_conn;

        returnQuery query_private(std::string& query, int size, const char* const* params);

    protected:
        ptrConnection getConnection();
        void freeConnection(ptrConnection connection);

    public:
        bool run(std::unique_ptr<iDBFacade::Configuration> config);
        void queryPrivate(std::string& query, std::vector<std::string>& param, returnQuery& result);

        Postgres() = default;
        virtual ~Postgres();

        friend class dbFacade;
    };
    using DataBase = Postgres;
}
#endif //FHTPOSTGRESQL_H
