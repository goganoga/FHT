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
        struct data {
            int id = 0; //port
            std::string str0; //uri
            std::string str1; //nextLocation
            std::string str2; //postBody
            std::string str3; //ip
            double num = 0;
            void* obj0;
            std::any obj1;
            std::map<std::string, std::string> map0; //params
            std::map<std::string, std::string> map1; //headers

        };
        using uniqueHendler = std::function<std::string(iHendler::data&)>;
        virtual void addUniqueHendler(std::string id, uniqueHendler func) = 0;
        virtual bool removeUniqueHendler(std::string id) = 0;
        virtual std::shared_ptr<uniqueHendler> getUniqueHendler(std::string id) = 0;

        virtual void addHendler(std::string id, std::function<void(void)> func) = 0;
        virtual bool removeHendler(std::string id) = 0;
        virtual std::shared_ptr<std::function<void(void)>> getHendler(std::string id) = 0;
    };
}
#endif //FHTIHENDLER_H
