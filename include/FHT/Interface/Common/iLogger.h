/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 07.05.20
*  Copyright (C) goganoga 2020
***************************************/
#ifndef FHTILOGGER_H
#define FHTILOGGER_H
#include <string>
#include <memory>

namespace FHT {
    struct iLogger{
        enum class Verbose{
            OFF,
            FATAL,
            ERR,
            WARN,
            INFO,
            DEBUG
        };
        virtual void setLevelVerboseLogging(const Verbose level) = 0;
        virtual void setFileLogging(const std::string fileName) = 0;

        // if second = 0 then log output to stdout
        virtual void setTimeDump(const int second) = 0;
    };
}
#endif // ILOGGER_H
