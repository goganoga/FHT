/******************************************************
*  Cloud WebAccess
*  http://gitlab.qq/cloudproject/backend-web-access.git
*  Created: 01.04.2020
*  Copyright (C) goganoga 2020
*******************************************************/
#include "Postgresql.h"
#include "Template.h"

#include "iController.h"

#include <algorithm>
#include <stdexcept>

namespace FHT {
    
    struct Connection {
        using ptrConnection = std::shared_ptr<PostgresConnection>;
        Connection(ptrConnection connection, std::function<void(ptrConnection)> destructor) :m_connection(connection), m_destructor(destructor){}
        ~Connection() { m_destructor(m_connection); }
        PGconn* get() {
            if (m_connection) {
                return m_connection->connection().get();
            }
            return nullptr;
        }
        ptrConnection getPtr() {
            if (m_connection) {
                return m_connection;
            }
            return nullptr;
        }
    private:
        ptrConnection m_connection;
        std::function<void(ptrConnection)> m_destructor;
    };

    template<typename ...Args>
    Postgres::returnQuery Postgres::Query(std::string query, Args const ...args) {
        returnQuery result;
        try {
            if (!m_isRun) {
                throw "iPostgres not runing!";
            }
            std::vector<std::string> parameters{ args... };

            std::vector<std::unique_ptr<char[]>> list;
            if (!parameters.empty()) {
                std::unique_ptr<char* []> paramValues(new char* [parameters.size()]);
                for (int i = 0; i < parameters.size(); i++) {
                    if (parameters[i].empty()) {
                        paramValues[i] = nullptr;
                    }
                    else {
                        std::unique_ptr<char[]> str(new char[parameters[i].size() + 1]);
                        std::strncpy(str.get(), parameters[i].c_str(), parameters[i].size() + 1);
                        list.push_back(std::move(str));
                        paramValues[i] = list.back().get();
                    }
                }
                result = queryPrivate(query, static_cast<int>(parameters.size()), paramValues.get());
            }
            else {
                result = queryPrivate(query, 0, nullptr);
            }
        }
        catch (const char* e) {
            std::vector<std::string> vector;
            vector.push_back(e);
            result.emplace("error", vector);
            FHT::LoggerStream::Log(FHT::LoggerStream::WARN) << METHOD_NAME << vector[0];
        }
        catch (...) {
            std::vector<std::string> vector;
            vector.push_back("unknown");
            result.emplace("error", vector);
            FHT::LoggerStream::Log(FHT::LoggerStream::ERR) << METHOD_NAME << "unknown";
        }

        FHT::LoggerStream::Log(FHT::LoggerStream::DEBUG) << METHOD_NAME << "ok";
        return result;
    }

    Postgres::returnQuery Postgres::queryPrivate(std::string& query, int size, const char* const* params) {
        returnQuery out;
        auto uuid =  "web_access_" + guid(query);
        uuid.erase(std::remove(uuid.begin(), uuid.end(), '-'), uuid.end());
        {
            std::unique_ptr<PGresult, decltype(&PQclear)> m_result{ nullptr, [](PGresult* res) { if (res) PQclear(res); } };
            Connection connection{ getConnection(), [&](ptrConnection a) { if (a) freeConnection(a); } };
            if (!connection.getPtr()) {
                throw std::string("Error Connection").c_str();
            }

            if (!connection.getPtr()->findStmt(uuid)) {
                m_result.reset(PQprepare(connection.get(), uuid.c_str(), query.c_str(), 0, nullptr));
                if (!m_result) {
                    throw std::string("Error create PG stmt").c_str();
                }
                if (PQresultStatus(m_result.get()) != PGRES_COMMAND_OK) {
                    throw PQresultErrorMessage(m_result.get());
                }
                connection.getPtr()->addStmt(uuid);
            }

            m_result.reset(PQexecPrepared(connection.get(), uuid.c_str(), size, params, nullptr, nullptr, 0));
            if (!m_result) {
                throw std::string("Error exec PG query trough stmt").c_str();
            }
            if (auto const status = PQresultStatus(m_result.get()); status != PGRES_TUPLES_OK && status != PGRES_COMMAND_OK) {
                throw PQresultErrorMessage(m_result.get());
            }
            if (PQntuples(m_result.get())) {
                for (int i = 0; i < PQnfields(m_result.get()); i++) {
                    std::vector<std::string> vector;
                    for (int j = 0; j < PQntuples(m_result.get()); j++) {
                        vector.push_back(PQgetvalue(m_result.get(), j, i));
                    }
                    out.emplace(PQfname(m_result.get(), i), vector);
                }
            }
        }
        return out;
    }

    Postgres::~Postgres() {
        m_isRun = false;
    }

    Postgres::ptrConnection Postgres::getConnection() {
        std::unique_lock<std::mutex> lock_(m_mux);
        for (; m_pool_conn.empty();) {
            m_condition.wait(lock_);
        }
        auto connection = m_pool_conn.front();
        m_pool_conn.pop();
        return connection;
    }

    void Postgres::freeConnection(ptrConnection connection) {
        std::unique_lock<std::mutex> lock_(m_mux);
        m_pool_conn.push(connection);
        lock_.unlock();
        m_condition.notify_one();
    }

    bool Postgres::run() {
        try {
            for (int i = 0; i < m_poolCount; i++) {
                m_pool_conn.emplace(std::make_shared<PostgresConnection>(m_port, m_host, m_name, m_user, m_pass));
            }
            FHT::iConrtoller::taskManager->addTask(FHT::iTask::UI, [&]() {
                auto a = Query("DELETE FROM notifications.notification WHERE status = 'true';");
                FHT::LoggerStream::Log(FHT::LoggerStream::DEBUG) << METHOD_NAME << "DELETE FROM notifications";
                return FHT::iTask::state::CONTINUE;
                },
            1000 * 60 * 60 * 8); // ms * s * m * h
            m_isRun = true;
            return true;
        }
        catch (std::exception const& e) {
            throw e;
        }
        return false;
    }

    void Postgres::setPort(int arg){
        FHT::LoggerStream::Log(FHT::LoggerStream::DEBUG) << METHOD_NAME << arg;
        m_port = arg;
    }

    void Postgres::setHost(std::string arg){
        FHT::LoggerStream::Log(FHT::LoggerStream::DEBUG) << METHOD_NAME << arg;
        m_host = arg;
    }

    void Postgres::setName(std::string arg){
        FHT::LoggerStream::Log(FHT::LoggerStream::DEBUG) << METHOD_NAME << arg;
        m_name = arg;
    }

    void Postgres::setUser(std::string arg){
        FHT::LoggerStream::Log(FHT::LoggerStream::DEBUG) << METHOD_NAME << arg;
        m_user = arg;
    }

    void Postgres::setPass(std::string arg){
        FHT::LoggerStream::Log(FHT::LoggerStream::DEBUG) << METHOD_NAME << arg;
        m_pass = arg;
    }

    void Postgres::setWorker(int arg) {
        FHT::LoggerStream::Log(FHT::LoggerStream::DEBUG) << METHOD_NAME << arg;
        m_poolCount = arg;
    }
    
}