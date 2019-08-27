/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 06.08.19
*  Copyright (C) goganoga 2019
***************************************/
#ifndef FHTICONTROLLER_H
#define FHTICONTROLLER_H
#include "../Common/Controller/controller.h"
namespace FHT{
    namespace iConrtoller {
        static auto hendlerManager = FHT::Conrtoller::getHendler();
        static auto taskManager = FHT::Conrtoller::getTask();
    }
}
#endif // FHTICONTROLLER_H
