/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 06.08.19
*  Copyright (C) goganoga 2019
***************************************/
#ifndef ICONTROLLER_H
#define ICONTROLLER_H
#include "controller.h"
namespace FHT{
    namespace iConrtoller {
        static auto hendlerManager = FHT::Conrtoller::getHendler();
        static auto taskManager = FHT::Conrtoller::getTask();
    }
}
#endif // ICONTROLLER_H
