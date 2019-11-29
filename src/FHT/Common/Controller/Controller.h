/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 06.08.19
*  Copyright (C) goganoga 2019
***************************************/
#ifndef FHTCONTROLLER_H
#define FHTCONTROLLER_H
#include "iTask.h"
#include "iServer.h"
#include "iClient.h"
#include "iHendler.h"
#include <memory>
namespace FHT{
    struct Conrtoller {
        static std::shared_ptr<FHT::iHendler> getHendler();
        static std::shared_ptr<FHT::iServer> getServer();
        static std::shared_ptr<FHT::iClient> getClient();
        static std::shared_ptr<FHT::iTask> getTask();
    };
}
#endif // FHTCONTROLLER_H
