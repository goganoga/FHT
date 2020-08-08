/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 05.06.20
*  Copyright (C) goganoga 2020
***************************************/
#ifndef FHTDBFACADE_H
#define FHTDBFACADE_H
#include "iDBFacade.h"

#ifdef DBPOSTGRESQL
#include "Postgresql/Postgresql.h"
#include "DBFacade/postgresConfiguration.h"
#elif DBNONE
    struct Configuration {
    };
    struct DataBase {
        template<typename...>
        bool run(...) { return false; };
        template<typename...>
        void queryPrivate(...) {};
    };
#endif

#include <memory>
#include <string>
#include <map>

namespace FHT {
    namespace iDBFacade {
        class dbFacade : virtual public iDBConnect, virtual public iDBSettings, public Details::dbFacadePtr {
            void setConfiguration(Configuration arg) override final;
            bool run() override final;
            void query(std::string& query, std::vector<std::string>& param, returnQuery& result) override final;

            std::shared_ptr<DataBase> db_ptr;
            std::unique_ptr<Configuration> m_config_ptr;
        public:
            dbFacade();
            virtual ~dbFacade() override;
        };
    }
}
#endif //FHTDBFACADE_H
