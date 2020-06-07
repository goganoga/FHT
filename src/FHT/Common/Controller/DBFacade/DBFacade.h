/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 05.06.20
*  Copyright (C) goganoga 2020
***************************************/
#ifndef FHTDBFACADE_H
#define FHTDBFACADE_H
#include "Common/iDBFacade.h"

#ifdef DBPOSTGRESQL
#include "Postgresql/Postgresql.h"
#endif

#include <memory>
#include <string>
#include <map>

namespace FHT {
    class dbFacade : public iDBFacade, public iDBConnect {
#ifdef DBPOSTGRESQL
        using DataBase = Postgres;
        returnQuery queryPrivate(std::string& query, int size, const char* const* params) override final;
#elif DBNONE
        struct DataBase {
            void setHost(std::string arg) {};
            void setName(std::string arg) {};
            void setUser(std::string arg) {};
            void setPass(std::string arg) {};
            void setPort(int arg) {};
            void setWorker(int arg) {};
            bool run() { return false; };
            template<typename ...Args>
            iDBFacade::returnQuery queryPrivate(Args const ...args) { return iDBFacade::returnQuery{}; };
        };
#endif

        void setHost(std::string arg) override final;
        void setName(std::string arg) override final;
        void setUser(std::string arg) override final;
        void setPass(std::string arg) override final;
        void setPort(int arg) override final;
        void setWorker(int arg) override final;
        bool run() override final;
        const std::shared_ptr<iDBConnect> operator->() override final;

        std::shared_ptr<DataBase> db_ptr;
        std::shared_ptr<dbFacade> shared_from_this;
        volatile bool isRun;
    public:
        dbFacade();
        virtual ~dbFacade() override;

    };
}
#endif //FHTDBFACADE_H
