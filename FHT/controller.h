/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 06.08.19
*  Copyright (C) goganoga 2019
***************************************/
#ifndef CONTROLLER_H
#define CONTROLLER_H
#include "iTask.h"
#include "iHendler.h"
#include <memory>
namespace FHT{
    struct Conrtoller {
        static std::shared_ptr<FHT::iHendler> getHendler();
        static std::shared_ptr<FHT::iTask> getTask();
	};
}
#endif // CONTROLLER_H
