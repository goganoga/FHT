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
    class dbFacade : public iDBFacade {
        std::map<std::string, std::shared_ptr<iDBFacade>> list_support_db = {
          //  {"postgres", std::make_shared<Postgres>()}
        };

        void setHost(std::string arg) override final;
        void setName(std::string arg) override final;
        void setUser(std::string arg) override final;
        void setPass(std::string arg) override final;
        void setPort(int arg) override final;
        void setWorker(int arg) override final;

        bool run() override final;

        std::shared_ptr<DB> m_db_ptr;
        std::shared_ptr<iDBFacade> m_dbFacade_ptr;

    public:
        std::shared_ptr<DB> operator->() override final;

        dbFacade() = default;
        virtual ~dbFacade() override;

    };
}
#endif //FHTDBFACADE_H
