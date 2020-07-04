/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 06.08.19
*  Copyright (C) goganoga 2019
***************************************/
#ifndef FHTICONTROLLER_H
#define FHTICONTROLLER_H
#include "../Common/Controller/Controller.h"
namespace FHT {
    namespace iConrtoller {
        inline auto logger = FHT::Conrtoller::getLogger();
        inline auto taskManager = FHT::Conrtoller::getTask();
        inline auto hendlerManager = FHT::Conrtoller::getHendler();
        inline auto webServer = FHT::Conrtoller::getServer();
    }
}
#endif // FHTICONTROLLER_H
