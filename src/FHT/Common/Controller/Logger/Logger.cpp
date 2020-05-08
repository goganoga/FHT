/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 07.05.20
*  Copyright (C) goganoga 2020
***************************************/
#include "Logger.h"
#include "Controller/Controller.h"
#include "../../../Interface/iController.h"
#include <string>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>

namespace {
    logWriter::logWriter(const std::string file_name):
        m_out(file_name, std::ios::app){};

    logWriter::~logWriter(){
        m_out.close();
    };

    void logWriter::writeFromList(const std::vector<std::string>& list){
        if (m_out.is_open()) {
            for(auto &a: list)
            m_out << a << std::endl;
        }
    };
}
namespace FHT {
    std::shared_ptr<Logger> Logger::gerPtrLogger() {
        static auto a = std::make_shared<Logger>();
        return a;
    }

    Logger::~Logger() {
        std::lock_guard<decltype(m_mutex)> lock(m_mutex);
        std::vector<std::string> list;
        list.swap(m_list);
        m_write->writeFromList(list);
        m_write.reset();
    }

    void Logger::setLevelVerboseLogging(const Verbose level) {
      m_verboseLogging = level;
    };

    void Logger::setFileLogging(const std::string fileName) {
      m_fileLoggingName = fileName;
    };

    void Logger::setTimeDump(const int second) {
      m_intervalSec = second;
    };

    LoggerStream Logger::log(const LoggerStream::Level level) {
        if(static_cast<int>(level) <= static_cast<int>(m_verboseLogging)){
            if(!m_write){
                m_write.reset(new logWriter(m_fileLoggingName));

                if(m_intervalSec){ 
                    FHT::iConrtoller::taskManager->addTask(FHT::iTask::IO,
                        [&, f_write = std::weak_ptr<logWriter>(m_write)] () mutable {
                        try {
                            if (auto func = f_write.lock(); func) {
                                std::vector<std::string> list;
                                {
                                    std::lock_guard<decltype(m_mutex)> lock(m_mutex);
                                    list.swap(m_list);
                                    func->writeFromList(list);
                                }
                                return FHT::iTask::state::CONTINUE;
                            }
                            return FHT::iTask::state::FINISH;
                        }
                        catch (...) {
                            return FHT::iTask::state::FINISH;
                        }
                    }, m_intervalSec * 1000);
                }
            }
            auto t = std::time(nullptr);
            auto tm = *std::localtime(&t);
            std::ostringstream oss;
            oss << std::put_time(&tm, "%d.%m.%Y %H:%M:%S");
            return LoggerStream(oss.str() + "|[" + m_level[static_cast<int>(level)] + "]",[&](const std::string& str){
                    std::lock_guard<decltype(m_mutex)> lock(m_mutex);
                    if(!m_intervalSec){
                        std::cout << str << std::endl;
                    } else {
                        m_list.push_back(str);
                    }
            });
        }
        return LoggerStream();
    };

    LoggerStream LoggerStream::Log(const LoggerStream::Level level){
        return Logger::gerPtrLogger()->log(level);

    }

    std::shared_ptr<iLogger> Conrtoller::getLogger() {
        return Logger::gerPtrLogger();
    }
}
