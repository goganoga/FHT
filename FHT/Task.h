/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 06.08.19
*  Copyright (C) goganoga 2019
***************************************/
#ifndef TASK_H
#define TASK_H
#include "iTask.h"
#include <utility>
#include <memory>
#include <map>

struct iThread {
	virtual ~iThread() = default;
	virtual void pull(std::function<void(void)> func) = 0;
	virtual void pull(std::function<void(void)> func, int ms) = 0;
    virtual void pullTime(std::function<void(void)> func, int ms) = 0;
    virtual bool isRun() { return false; }


};

std::shared_ptr<iThread> makeThread();

namespace FHT{
    class Task: public iTask {
    public:
        Task();
        virtual ~Task() override;
        void addTask(iTask::listTask thread, std::function<void(void)> func) override final;
        void addTask(iTask::listTask thread, std::function<void(void)> func, int ms) override final;
        void addTaskOneRun(iTask::listTask thread, std::function<void(void)> func, int ms) override final;
        bool startManager() override final;
        bool stopManager() override final;
    private:
        std::shared_ptr<iThread> getTaskThread(iTask::listTask thread);
        std::map<std::size_t, std::shared_ptr<iThread>> factory_;
        template <std::size_t ... I>
        static std::map<std::size_t, std::shared_ptr<iThread>> make_factory(std::index_sequence<I ... > const &);
    };
}
#endif //TASK_H
