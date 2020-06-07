/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 05.06.20
*  Copyright (C) goganoga 2020
***************************************/
#ifndef FHTIDBFACADE_H
#define FHTIDBFACADE_H
#include <map>
#include <string>
#include <vector>
#include <memory>

namespace FHT {
    struct iDBFacade {
        virtual ~iDBFacade() = default;
        using returnQuery = std::map<std::string, std::vector<std::string>>;
        template<typename ...Args>
        returnQuery Query(std::string query, Args const ...args) {
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

        virtual void setHost(std::string arg) = 0;
        virtual void setName(std::string arg) = 0;
        virtual void setUser(std::string arg) = 0;
        virtual void setPass(std::string arg) = 0;
        virtual void setPort(int arg) = 0;
        virtual void setWorker(int arg) = 0;
        virtual bool run() { return false; };
    };
}
#endif //FHTIDBFACADE_H
