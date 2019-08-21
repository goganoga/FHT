#ifndef CONTROLLER_H
#define CONTROLLER_H
#include "iTask.h"
#include "iHendler.h"
#include <memory>
namespace Frame{
    struct Conrtoller {
		static std::shared_ptr<iHendler> getHendler();
		static std::shared_ptr<iTask> getTask();
	};
}
#endif // CONTROLLER_H
