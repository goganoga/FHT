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
        static auto hendlerManager = FHT::Conrtoller::getHendler();
        static auto taskManager = FHT::Conrtoller::getTask();
        static auto webServer = FHT::Conrtoller::getServer();
        static auto webClient = FHT::Conrtoller::getClient();
        static auto logger = FHT::Conrtoller::getLogger();
    }
}
#endif // FHTICONTROLLER_H
