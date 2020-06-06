/******************************************************
*  Cloud WebAccess
*  http://gitlab.qq/cloudproject/backend-web-access.git
*  Created: 09.05.2020
*  Copyright (C) goganoga 2020
*******************************************************/
#include "PostgresConnection.h"

#include "LoggerStream.h"
#include <stdexcept>

namespace FHT {
    PostgresConnection::PostgresConnection(const int& m_port, const std::string& m_host, const std::string& m_name, const std::string& m_user, const std::string& m_pass) {
        if (!m_host.empty() && !m_name.empty() && !m_user.empty() && !m_pass.empty()) {
            FHT::LoggerStream::Log(FHT::LoggerStream::DEBUG) << METHOD_NAME;
            m_connection.reset(PQsetdbLogin(m_host.c_str(), std::to_string(m_port).c_str(), nullptr, nullptr, m_name.c_str(), m_user.c_str(), m_pass.c_str()), &PQfinish);
            if (PQstatus(m_connection.get()) != CONNECTION_OK && PQsetnonblocking(m_connection.get(), 1) != 0){
                throw std::runtime_error(PQerrorMessage(m_connection.get()));
            }
        }
        else {
            throw std::runtime_error("Postgres error config!!!");
        }
    };

    PostgresConnection::~PostgresConnection() {
        std::unique_ptr<PGresult, decltype(&PQclear)> m_result{ nullptr, [](PGresult* res) { if (res) PQclear(res); } };
        for (auto& a : m_stmt_list) {
            std::string command{ "DEALLOCATE " + a };
            m_result.reset(PQexec(m_connection.get(), command.c_str()));
            if (!m_result) {
                FHT::LoggerStream::Log(FHT::LoggerStream::FATAL) << METHOD_NAME << std::string("Error exec PG DEALLOCATE stmt");
            }
            if (PQresultStatus(m_result.get()) != PGRES_COMMAND_OK) {
                FHT::LoggerStream::Log(FHT::LoggerStream::ERR) << METHOD_NAME << std::string(PQresultErrorMessage(m_result.get()));
            }
        }
        m_connection.reset();
    };

    const std::shared_ptr<PGconn> PostgresConnection::connection() {
        return m_connection;
    };

    void PostgresConnection::addStmt(std::string& uuid) {
        m_stmt_list.emplace(uuid);
    };

    bool PostgresConnection::findStmt(std::string& uuid) {
        auto a = m_stmt_list.find(uuid);
        return a != end(m_stmt_list);
    };
};