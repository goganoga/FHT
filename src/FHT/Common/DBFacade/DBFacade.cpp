/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 05.06.20
*  Copyright (C) goganoga 2020
***************************************/
#include "FHT/Common/DBFacade/DBFacade.h"
#include "FHT/Common/Controller/Controller.h"
#include "FHT/Interface/DBFacade/iDBFacade.h"
#include "FHT/LoggerStream.h"

namespace FHT {
    namespace iDBFacade {

        void dbFacade::setConfiguration(Configuration arg) {
            m_config_ptr.reset(new Configuration(arg));
        }

        bool dbFacade::run() {
            if (!m_config_ptr) {
                FHT::LoggerStream::Log(FHT::LoggerStream::FATAL) << METHOD_NAME << "Already run or you need call setConfiguration";
                return false;
            }
            return db_ptr->run(std::move(m_config_ptr));
        }

        void dbFacade::query(std::string& query, std::vector<std::string>& param, iDBConnect::returnQuery& result) {
            db_ptr->queryPrivate(query, param, result);
        }

        dbFacade::dbFacade() :
            db_ptr(std::make_shared<DataBase>()){
        }

        dbFacade::~dbFacade() {}

        std::shared_ptr<Details::dbFacadePtr> Details::dbFacadePtr::Get() {
            static auto a = std::make_shared<dbFacade>();
            return a;
        }
    }
}
