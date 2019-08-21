/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 06.08.19
*  Copyright (C) goganoga 2019
***************************************/
#ifndef ICONTROLLER_H
#define ICONTROLLER_H
#include "controller.h"

namespace Conrtoller {
    static auto hendlerManager = Frame::Conrtoller::getHendler();
    static auto taskManager = Frame::Conrtoller::getTask();
}
#endif // ICONTROLLER_H
