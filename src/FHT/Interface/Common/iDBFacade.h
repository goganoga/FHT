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
    struct DB {
        using returnQuery = std::map<std::string, std::vector<std::string>>;
        template<typename ...Args>
        returnQuery Query(std::string query, Args const ...args) {
            return returnQuery{};
        };
        void RRR() {};
    };
    struct iDBFacade {

        virtual std::shared_ptr<DB> operator->() {
            return std::make_shared<DB>();
        };

        virtual ~iDBFacade() = default;
    protected:
        //postgres
        virtual void setDBType(std::string arg) {};

        virtual void setHost(std::string arg) = 0;
        virtual void setName(std::string arg) = 0;
        virtual void setUser(std::string arg) = 0;
        virtual void setPass(std::string arg) = 0;
        virtual void setPort(int arg) = 0;
        virtual void setWorker(int arg) = 0;

        virtual bool run() { return false; };
        friend struct inizialaizer;
        friend class dbFacade;
    };
}
#endif //FHTIDBFACADE_H
