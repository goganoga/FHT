/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 14.10.19
*  Copyright (C) goganoga 2019
***************************************/
#ifndef FHTICLIENT_H
#define FHTICLIENT_H
#include <memory>
#include <cstdint>
#include <functional>
#include <map>

namespace FHT {
    struct iClient {
        struct httpClient {
            struct httpResponse {
                int status;
                std::string body;
                std::map<std::string, std::string> headers;
            };
            enum class Type {
                GET,
                POST
            };
            std::string url;
            std::string body;
            std::map<std::string, std::string> headers;
            bool body_is_file_name = false;
            Type type = Type::GET;

            //async fetch
            void fetch(std::function<void(httpResponse)>);

            //sync fetch
            const httpResponse fetch();
        };
        virtual ~iClient() = default;
    };
}
#endif //FHTICLIENT_H
