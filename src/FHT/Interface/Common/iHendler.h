/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 06.08.19
*  Copyright (C) goganoga 2019
***************************************/
#ifndef FHTIHENDLER_H
#define FHTIHENDLER_H
#include <functional>
#include <string>
#include <cstring>
#include <memory>
#include <any>
#include <map>
namespace FHT{
    struct iHendler {
        virtual ~iHendler() = default;
        struct dataRequest {
            std::map<std::string, std::string> params;
            std::map<std::string, std::string> headers;
            std::string uri;
            std::string nextLocation;
            std::string ipClient;
            int portClient = 0;

            size_t sizeBody;
            std::string typeBody;
            std::shared_ptr<char> body;

            std::any WSInstanse;
            void* ptr;
        };
        struct dataResponse {
            size_t sizeBody;
            std::string typeBody = "text/plain";
            std::shared_ptr<char> body;
        };
        using uniqueHendler = std::function<dataResponse(iHendler::dataRequest&)>;
        virtual void addUniqueHendler(std::string id, uniqueHendler func) = 0;
        virtual bool removeUniqueHendler(std::string id) = 0;
        virtual std::shared_ptr<uniqueHendler> getUniqueHendler(std::string id) = 0;

        virtual void addHendler(std::string id, std::function<void(void)> func) = 0;
        virtual bool removeHendler(std::string id) = 0;
        virtual std::shared_ptr<std::function<void(void)>> getHendler(std::string id) = 0;
    };
}
#endif //FHTIHENDLER_H
