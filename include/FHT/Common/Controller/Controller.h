/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 06.08.19
*  Copyright (C) goganoga 2019
***************************************/
#ifndef FHTCONTROLLER_H
#define FHTCONTROLLER_H
#include "FHT/Interface/Common/iTask.h"
#include "FHT/Interface/Common/iServer.h"
#include "FHT/Interface/Common/iClient.h"
#include "FHT/Interface/Common/iHendler.h"
#include "FHT/Interface/Common/iLogger.h"
#include <memory>

namespace FHT{
    struct Conrtoller {
        static std::shared_ptr<FHT::iLogger> getLogger();
        static std::shared_ptr<FHT::iTask> getTask();
        static std::shared_ptr<FHT::iHendler> getHendler();
        static std::shared_ptr<FHT::iServer> getServer();
    };
}
#endif // FHTCONTROLLER_H
