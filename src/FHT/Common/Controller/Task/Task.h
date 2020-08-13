/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 06.08.19
*  Copyright (C) goganoga 2019
***************************************/
#ifndef FHTTASK_H
#define FHTTASK_H
#include "Common/iTask.h"
#include <atomic>
#include <chrono>
#include <utility>
#include <memory>
#include <map>

struct iThread {
    virtual ~iThread() = default;
    virtual void push(std::function<FHT::iTask::state(void)> func, int ms) = 0;
    virtual bool isRun() { return false; }
};

namespace FHT{
    class Task: public iTask {
        bool startManager();
        bool stopManager();
        std::shared_ptr<iThread> getTaskThread(iTask::listTask thread);
    public:
        Task();
        virtual ~Task() override;
        void postLoopTask(iTask::listTask thread, std::function<state(void)> func, int ms = 0) override final;
        void postTask(iTask::listTask thread, std::function<void(void)> func, int ms = 0) override final;
    private:
        template <std::size_t ... I>
        static std::map<std::size_t, std::shared_ptr<iThread>> make_factory(std::index_sequence<I ... > const &);
        static std::shared_ptr<iThread> makeThread();

        bool volatile isRun = false;
        std::map<std::size_t, std::shared_ptr<iThread>> factory_;
    };
}
#endif //FHTTASK_H
