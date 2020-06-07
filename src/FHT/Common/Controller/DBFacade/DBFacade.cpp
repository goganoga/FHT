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
        db_ptr = std::make_shared<DataBase>();
        isRun = db_ptr->run();
        return isRun;
    }

    dbFacade::~dbFacade() {}

    template<typename ...Args>
    iDBFacade::returnQuery iDBFacade::Query(std::string query, Args const ...args) {
        if (!isRun) {
            FHT::LoggerStream::Log(FHT::LoggerStream::FATAL) << METHOD_NAME << "Need runnig FHT::iConrtoller::dbFacade";
        }
        returnQuery result;
        try {
            std::vector<std::string> parameters{ args... };
            std::shared_ptr<DBFacade> db_ptr = Conrtoller::getDBFacade();
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
                result = db_ptr->queryPrivate(query, static_cast<int>(parameters.size()), paramValues.get());
            }
            else {
                result = db_ptr->queryPrivate(query, 0, nullptr);
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

    std::shared_ptr<iDBFacade> Conrtoller::getDBFacade() {
        auto static a = std::make_shared<dbFacade>();
        return a;
    }
}
