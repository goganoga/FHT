/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 06.08.19
*  Copyright (C) goganoga 2019
***************************************/
#ifndef FHTCONTROLLER_H
#define FHTCONTROLLER_H
#include "Common/iTask.h"
#include "Common/iServer.h"
#include "Common/iClient.h"
#include "Common/iHendler.h"
#include "Common/iLogger.h"
#include "Common/iDBFacade.h"
#include <memory>
namespace FHT{
    struct Conrtoller {
        static std::shared_ptr<FHT::iLogger> getLogger();
        static std::shared_ptr<FHT::iTask> getTask();
        static std::shared_ptr<FHT::iHendler> getHendler();
        static std::shared_ptr<FHT::iServer> getServer();
        static std::shared_ptr<FHT::iClient> getClient();
        static std::shared_ptr<FHT::iDBFacade> getDBFacade();
    };
}
#endif // FHTCONTROLLER_H
