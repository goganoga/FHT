/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 05.06.20
*  Copyright (C) goganoga 2020
***************************************/
#ifndef FHTIDBFACADE_H
#define FHTIDBFACADE_H
#include "LoggerStream.h"

#include <map>
#include <string>
#include <vector>
#include <memory>

namespace FHT {
    struct iDBConnect {
        using returnQuery = std::map<std::string, std::vector<std::string>>;
        virtual ~iDBConnect() = default;
        template<typename ...Args>
        returnQuery Query(std::string query_str, Args const ...args) {
            returnQuery result;
            try {
                std::vector<std::string> parameters{ args... };
                query(query_str, parameters, result);
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
    private:
        void query(std::string& query, std::vector<std::string>& param, returnQuery& result);

#ifdef DBPOSTGRESQL
        virtual returnQuery queryPrivate(std::string& query, int size, const char* const* params) { return returnQuery{}; };
#endif
    };

    struct iDBFacade {
        virtual ~iDBFacade() = default;
        virtual void setHost(std::string arg) {};
        virtual void setName(std::string arg) {};
        virtual void setUser(std::string arg) {};
        virtual void setPass(std::string arg) {};
        virtual void setPort(int arg) {};
        virtual void setWorker(int arg) {};
        virtual bool run() { return false; };
        virtual const iDBConnect* getConnector() { return nullptr; }
    };
}
#endif //FHTIDBFACADE_H
