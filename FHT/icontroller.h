#ifndef ICONTROLLER_H
#define ICONTROLLER_H
#include "controller.h"

namespace Conrtoller {
    static auto hendlerManager = Frame::Conrtoller::getHendler();
    static auto taskManager = Frame::Conrtoller::getTask();
}
#endif // ICONTROLLER_H
