/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 07.05.20
*  Copyright (C) goganoga 2020
***************************************/
#ifndef FHTLOGGERSTREAM_H
#define FHTLOGGERSTREAM_H
#include <functional>
#include <string>
#include <sstream>

namespace {
    inline const std::string className(const std::string& prettyFunction)
    {
        size_t beginmehod = prettyFunction.rfind("(") + 1;
        size_t begin = prettyFunction.rfind(" ", beginmehod) + 1;
        return prettyFunction.substr(begin, prettyFunction.size());
    }
}

#if defined(__GNUC__) || defined(__clang__)
#define METHOD_NAME className(__PRETTY_FUNCTION__).c_str()
#elif defined(_MSC_VER)
#define METHOD_NAME className(__FUNCSIG__).c_str()
#else
#define METHOD_NAME className(__func__).c_str()
#endif

namespace FHT {
    class LoggerStream {
        std::string m_buf;
        const std::function<void(const std::string&)> m_bind;
    public:
        LoggerStream(const std::string str, const std::function<void(const std::string&)> bind): m_buf(str)/*, m_ptr(this)*/, m_bind(bind){};
        LoggerStream(){};
        ~LoggerStream(){
            if(m_bind){
                m_bind(m_buf);
            }
        };

        template<typename T>
        LoggerStream& operator << (const T str) {
            std::stringstream ss;
            ss << "|" <<  str;
            m_buf.append(ss.str());
            return *this;
        };

        enum Level{
            FATAL,
            ERR,
            WARN,
            INFO ,
            DEBUG
        };
        static LoggerStream Log(const Level level);
    };

}
#endif // FHTLOGGERSTREAM_H
