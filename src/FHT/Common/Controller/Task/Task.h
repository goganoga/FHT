/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 06.08.19
*  Copyright (C) goganoga 2019
***************************************/
#ifndef FHTTASK_H
#define FHTTASK_H
#include "iTask.h"
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
        void addTask(iTask::listTask thread, std::function<state(void)> func) override final;
        void addTask(iTask::listTask thread, std::function<state(void)> func, int ms) override final;
        void addTaskOneRun(iTask::listTask thread, std::function<void(void)> func) override final;
        void addTaskOneRun(iTask::listTask thread, std::function<void(void)> func, int ms) override final;
        void setDeltaTime(std::chrono::microseconds delta_time) override final;
    private:
        template <std::size_t ... I>
        static std::map<std::size_t, std::shared_ptr<iThread>> make_factory(std::atomic<std::chrono::microseconds> &delta_time, std::index_sequence<I ... > const &);
        static std::shared_ptr<iThread> makeThread(std::atomic<std::chrono::microseconds> &delta_time);

        bool volatile isRun = false;
        std::atomic<std::chrono::microseconds> delta_time_;
        std::map<std::size_t, std::shared_ptr<iThread>> factory_;
    };
}
#endif //FHTTASK_H
