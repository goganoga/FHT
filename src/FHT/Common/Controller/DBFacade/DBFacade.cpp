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

    iDBConnect::returnQuery dbFacade::queryPrivate(std::string& query, int size, const char* const* params) {
        return db_ptr->queryPrivate(query, size, params);
    };

    dbFacade::dbFacade():
        db_ptr(std::make_shared<DataBase>()),
        shared_from_this(std::make_shared<dbFacade>(*this)){}

    dbFacade::~dbFacade() {}

    const std::shared_ptr<iDBConnect> dbFacade::operator->() {
        return shared_from_this;
    }

    std::shared_ptr<iDBFacade> Conrtoller::getDBFacade() {
        auto static a = std::make_shared<dbFacade>();
        return a;
    }
}
