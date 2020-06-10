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
    namespace iDBFacade {
        struct Configuration;

        struct iDBSettings {
            virtual ~iDBSettings() = default;
            virtual void setConfiguration(Configuration arg) = 0;
            virtual bool run() = 0;
        };

        struct iDBConnect {
            using returnQuery = std::map<std::string, std::vector<std::string>>;

            virtual ~iDBConnect() = default;
            template<typename ...Args>
            inline returnQuery Query(std::string query_str, Args const ...args) {
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
            virtual void query(std::string& query, std::vector<std::string>& param, returnQuery& result) = 0;
        };
        
        namespace Details {
            struct dbFacadePtr : virtual public iDBSettings, virtual public iDBConnect {
                static std::shared_ptr<dbFacadePtr> Get();
            };
        }

        template<typename C>
        static std::shared_ptr<C> DBConnector() {
            return Details::dbFacadePtr::Get();
        }
    }
}
#endif //FHTIDBFACADE_H
