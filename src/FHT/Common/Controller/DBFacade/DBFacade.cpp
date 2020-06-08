/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 05.06.20
*  Copyright (C) goganoga 2020
***************************************/
#include "DBFacade.h"
#include "Common/iDBFacade.h"
#include "Controller/Controller.h"
#include "LoggerStream.h"

namespace FHT {
    void dbFacade::setHost(std::string arg) {
        if (isRun) {
            FHT::LoggerStream::Log(FHT::LoggerStream::FATAL) << METHOD_NAME << "Already run";
            return;
        }
        db_ptr->setHost(arg);
    }

    void dbFacade::setName(std::string arg) {
        if (isRun) {
            FHT::LoggerStream::Log(FHT::LoggerStream::FATAL) << METHOD_NAME << "Already run";
            return;
        }
        db_ptr->setName(arg);
    }

    void dbFacade::setUser(std::string arg) {
        if (isRun) {
            FHT::LoggerStream::Log(FHT::LoggerStream::FATAL) << METHOD_NAME << "Already run";
            return;
        }
        db_ptr->setUser(arg);
    }

    void dbFacade::setPass(std::string arg) {
        if (isRun) {
            FHT::LoggerStream::Log(FHT::LoggerStream::FATAL) << METHOD_NAME << "Already run";
            return;
        }
        db_ptr->setPass(arg);
    }

    void dbFacade::setPort(int arg) {
        if (isRun) {
            FHT::LoggerStream::Log(FHT::LoggerStream::FATAL) << METHOD_NAME << "Already run";
            return;
        }
        db_ptr->setPort(arg);
    }

    void dbFacade::setWorker(int arg) {
        if (isRun) {
            FHT::LoggerStream::Log(FHT::LoggerStream::FATAL) << METHOD_NAME << "Already run";
            return;
        }
        db_ptr->setWorker(arg);
    }

    bool dbFacade::run() {
        if (isRun) {
            FHT::LoggerStream::Log(FHT::LoggerStream::FATAL) << METHOD_NAME << "Already run";
            return false;
        }
        isRun = db_ptr->run();
        return isRun;
    }
    
    void iDBConnect::query(std::string& query, std::vector<std::string>& param, iDBConnect::returnQuery& result) {
#ifdef DBPOSTGRESQL
        std::vector<std::unique_ptr<char[]>> list;
        if (!param.empty()) {
            std::unique_ptr<char* []> paramValues(new char* [param.size()]);
            for (int i = 0; i < param.size(); i++) {
                if (param[i].empty()) {
                    paramValues[i] = nullptr;
                }
                else {
                    std::unique_ptr<char[]> str(new char[param[i].size() + 1]);
                    std::strncpy(str.get(), param[i].c_str(), param[i].size() + 1);
                    list.push_back(std::move(str));
                    paramValues[i] = list.back().get();
                }
            }
            result = queryPrivate(query, static_cast<int>(param.size()), paramValues.get());
        }
        else {
            result = queryPrivate(query, 0, nullptr);
        }
#endif
    }

#ifdef DBPOSTGRESQL
    iDBConnect::returnQuery dbFacade::queryPrivate(std::string& query, int size, const char* const* params) {
        return db_ptr->queryPrivate(query, size, params);
    };
#endif

    dbFacade::dbFacade():
        db_ptr(std::make_shared<DataBase>()),
        isRun(false) {}

    dbFacade::~dbFacade() {}

    const iDBConnect* dbFacade::getConnector() {
        if (!isRun) {
            FHT::LoggerStream::Log(FHT::LoggerStream::FATAL) << METHOD_NAME << "Need run as \"FHT::Conrtoller::getDBFacade()->run();\"";
            return nullptr;
        }
        const iDBConnect* a = static_cast<const iDBConnect*>(this);
        return a;
    }

    std::shared_ptr<iDBFacade> Conrtoller::getDBFacade() {
        static auto a = std::make_shared<dbFacade>();
        return a;
    }
}
