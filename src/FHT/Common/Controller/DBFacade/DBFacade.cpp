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
        if (!m_dbFacade_ptr) {
            FHT::LoggerStream::Log(FHT::LoggerStream::FATAL) << METHOD_NAME << "type DB not specified";
            return;
        }
        m_dbFacade_ptr->setHost(arg);
    }

    void dbFacade::setName(std::string arg) {
        if (!m_dbFacade_ptr) {
            FHT::LoggerStream::Log(FHT::LoggerStream::FATAL) << METHOD_NAME << "type DB not specified";
            return;
        }
        m_dbFacade_ptr->setName(arg);
    }

    void dbFacade::setUser(std::string arg) {
        if (!m_dbFacade_ptr) {
            FHT::LoggerStream::Log(FHT::LoggerStream::FATAL) << METHOD_NAME << "type DB not specified";
            return;
        }
        m_dbFacade_ptr->setUser(arg);
    }

    void dbFacade::setPass(std::string arg) {
        if (!m_dbFacade_ptr) {
            FHT::LoggerStream::Log(FHT::LoggerStream::FATAL) << METHOD_NAME << "type DB not specified";
            return;
        }
        m_dbFacade_ptr->setPass(arg);
    }

    void dbFacade::setPort(int arg) {
        if (!m_dbFacade_ptr) {
            FHT::LoggerStream::Log(FHT::LoggerStream::FATAL) << METHOD_NAME << "type DB not specified";
            return;
        }
        m_dbFacade_ptr->setPort(arg);
    }

    void dbFacade::setWorker(int arg) {
        if (!m_dbFacade_ptr) {
            FHT::LoggerStream::Log(FHT::LoggerStream::FATAL) << METHOD_NAME << "type DB not specified";
            return;
        }
        m_dbFacade_ptr->setWorker(arg);
    }

    bool dbFacade::run() {
        if (!m_dbFacade_ptr) {
            FHT::LoggerStream::Log(FHT::LoggerStream::FATAL) << METHOD_NAME << "type DB not specified";
            return false;
        }
        return m_dbFacade_ptr->run();
    }

    dbFacade::~dbFacade() {}

    std::shared_ptr<DB> dbFacade::operator-> () {
        if (!m_dbFacade_ptr) {
            FHT::LoggerStream::Log(FHT::LoggerStream::FATAL) << METHOD_NAME << "type DB not specified";
            return {};
        }
        return m_db_ptr;
    }

    std::shared_ptr<iDBFacade> Conrtoller::getDBFacade() {
        auto static a = std::make_shared<dbFacade>();
        return a;
    }
}
