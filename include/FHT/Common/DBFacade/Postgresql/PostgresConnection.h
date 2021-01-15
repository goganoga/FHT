/******************************************************
*  Cloud WebAccess
*  http://gitlab.qq/cloudproject/backend-web-access.git
*  Created: 09.05.2020
*  Copyright (C) goganoga 2020
*******************************************************/
#ifndef WEBSERVICESPOSTGRESQLCONNECTION_H
#define WEBSERVICESPOSTGRESQLCONNECTION_H
#include <memory>
#include <string>
#include <unordered_set>

#include <libpq-fe.h>

namespace FHT {
    class PostgresConnection {
        std::shared_ptr<PGconn> m_connection;
        std::unordered_set<std::string> m_stmt_list;
    public:
        PostgresConnection(const int& m_port, const std::string& m_host, const std::string& m_name, const std::string& m_user, const std::string& m_pass);
        ~PostgresConnection();
        const std::shared_ptr<PGconn> connection();
        void addStmt(std::string& uuid);
        bool findStmt(std::string& uuid);
    };
}
#endif //WEBSERVICESPOSTGRESQLCONNECTION_H
