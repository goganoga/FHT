/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 07.05.20
*  Copyright (C) goganoga 2020
***************************************/
#ifndef FHTLOGGER_H
#define FHTLOGGER_H
#include "FHT/Interface/Common/iLogger.h"
#include "FHT/LoggerStream.h"

#include <vector>
#include <mutex>
#include <fstream>
namespace {
    struct logWriter{
        logWriter(const std::string file_name);
        ~logWriter();
        void writeFromList(const std::vector<std::string>& list);
    private:
        const std::string file_name_;
    };
}
namespace FHT {
    class Logger : public iLogger{
        std::shared_ptr<logWriter> m_write;
        std::mutex m_mutex;
        std::vector<std::string> m_list;
        std::vector<std::string> m_level{ "FATAL", "ERROR", "WARNING", "INFO", "DEBUG" };
        Verbose m_verboseLogging = Verbose::OFF;
        std::string m_fileLoggingName = "FHT.log";
        int m_intervalSec = 0;
    public:
        static std::shared_ptr<Logger> gerPtrLogger();

        Logger() = default;
        virtual ~Logger();

        void setLevelVerboseLogging(const Verbose level) override final;
        void setFileLogging(const std::string fileName) override final;
        void setTimeDump(const int second) override final;
        LoggerStream log(const LoggerStream::Level level);

    };

}
#endif // FHTLOGGER_H
