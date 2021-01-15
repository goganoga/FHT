/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 05.06.20
*  Copyright (C) goganoga 2020
***************************************/
#ifndef FHTDBFACADE_H
#define FHTDBFACADE_H
#include "FHT/Interface/DBFacade/iDBFacade.h"

#ifdef DBPOSTGRESQL
#include "FHT/Common/DBFacade/Postgresql/Postgresql.h"
#include "FHT/Interface/DBFacade/postgresConfiguration.h"
#elif DBSQLITE
#include "FHT/Common/DBFacade/SQLite/SQLite.h"
#include "FHT/Interface/DBFacade/sqliteConfiguration.h"
#elif DBNONE

namespace FHT {
    namespace iDBFacade {
        struct Configuration {
        };
        struct DataBase {
            template<typename...>
            bool run(...) { return false; };
            template<typename...>
            void queryPrivate(...) {};
        };
    }
}
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
