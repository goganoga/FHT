#ifndef ITASK_H
#define ITASK_H
#include <functional>
class Task;
struct iTask {
	virtual ~iTask() = default;
	enum listTask {
		MAIN,
        MATH,
		IO,
        UI,
		size
	};
	virtual void addTask(iTask::listTask thread, std::function<void(void)> func) = 0;
	virtual void addTask(iTask::listTask thread, std::function<void(void)> func, int ms) = 0;
    virtual void addTaskOneRun(iTask::listTask thread, std::function<void(void)> func, int ms) = 0;
	virtual bool startManager() = 0;
    virtual bool stopManager() = 0;
};
#endif //ITASK_H
