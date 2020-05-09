/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 06.08.19
*  Copyright (C) goganoga 2019
***************************************/
#include "Task.h"
#include "Controller/Controller.h"
#include "Log/LoggerStream.h"

#include <chrono>
#include <thread>
#include <mutex>
#include <utility>
#include <map>
#include <iostream>
#include <queue>
#include <tuple>
#include <memory>

namespace FHT{
    Task::Task(){
        startManager();
    }
    Task::~Task(){
        stopManager();
    }

    void Task::addTask(iTask::listTask thread, std::function<state(void)> func) {
        getTaskThread(thread)->pull(func);
    }
    void Task::addTask(iTask::listTask thread, std::function<state(void)> func, int ms) {
        getTaskThread(thread)->pull(func, ms);
    }
    void Task::addTaskOneRun(iTask::listTask thread, std::function<void(void)> func) {
        getTaskThread(thread)->pullTime([func]() {func(); return FHT::iTask::state::FINISH; });
    }
    void Task::addTaskOneRun(iTask::listTask thread, std::function<void(void)> func, int ms) {
        getTaskThread(thread)->pullTime([func]() {func(); return FHT::iTask::state::FINISH; }, ms);
    }

    std::shared_ptr<iThread> Task::getTaskThread(iTask::listTask thread) {
        if(auto a = factory_.find(static_cast<std::size_t>(thread)); end(factory_) != a)
            return a->second;
        return nullptr;
    }
    bool Task::startManager() {
        factory_ = Task::make_factory(std::make_index_sequence<iTask::listTask::size>{});
        if(!factory_.empty())
            return true;
        return false;
    }
    bool Task::stopManager() {
        factory_.clear();
        if(factory_.empty())
            return true;
        return false;
    }
}
class Thread : public iThread {
    std::mutex mutex;
    using Threader = std::unique_ptr<std::thread, std::function<void(std::thread *)>>;
    Threader thread_{ nullptr, [](std::thread *a) { if (a) a->join(); delete a; } };
    bool volatile isRun_ = true;
    using tuple_ = std::tuple<std::function<FHT::iTask::state(void)>, long long, bool, decltype(std::chrono::high_resolution_clock::now())>;
    std::queue<tuple_> queue_;
    std::queue<tuple_> queue_buf_;
    std::chrono::microseconds sleep_;
public:
    virtual ~Thread() {
        isRun_ = false;
        thread_.reset();
    }
    Thread(): sleep_(std::chrono::microseconds(1000)){
        thread_.reset(new std::thread{ &Thread::loop, this });
    }
    Thread(std::chrono::microseconds sleep) : sleep_(sleep) {
        thread_.reset(new std::thread{ &Thread::loop, this });
    }
    enum tuple{
        function,
        ms,
        isLoop,
        ts
    };
    void loop() {
        while (isRun_) {
            try {
                tuple_ a;
                bool find = false;
                try {
                    const std::lock_guard<decltype(mutex)> lock(mutex);
                    if (!queue_.empty()) {
                        a = queue_.front();
                        queue_.pop();
                        find = true;
                    }
                    else {
                        if (!queue_buf_.empty()) {
                            queue_.swap(queue_buf_);
                        }
                    }
                }
                catch (std::exception e) {
                    FHT::LoggerStream::Log(FHT::LoggerStream::ERR) << METHOD_NAME << e.what();
                }
                if (find) {
                    auto realtime = std::chrono::high_resolution_clock::now();
                    auto& timestamp = std::get<tuple::ts>(a);
                    auto& timerun = std::get<tuple::ms>(a);
                    auto difftime(std::chrono::duration_cast<std::chrono::milliseconds>(realtime - timestamp));
                    if (difftime.count() < timerun) {
                        queue_buf_.push(a);
                    }
                    else {
                        auto& functor = std::get<tuple::function>(a);
                        if (functor && functor() == FHT::iTask::state::CONTINUE && std::get<tuple::isLoop>(a)) {
                            timestamp = std::chrono::high_resolution_clock::now();
                            queue_buf_.push(a);
                        }
                    }
                    std::this_thread::sleep_for(std::chrono::microseconds(100));
                }
                else {
                    std::this_thread::sleep_for(sleep_);
                }
            } catch (std::exception const& e) {
                FHT::LoggerStream::Log(FHT::LoggerStream::ERR) << METHOD_NAME << e.what();
            }
        }
    }
    void pull(std::function<FHT::iTask::state(void)> func) {
        const std::lock_guard<decltype(mutex)> lock(mutex);
        queue_.push(std::make_tuple(func, 0, true, std::chrono::high_resolution_clock::now()));
    }
    void pull(std::function<FHT::iTask::state(void)> func, int ms) {
        const std::lock_guard<decltype(mutex)> lock(mutex);
        queue_.push(std::make_tuple(func, ms, true, std::chrono::high_resolution_clock::now()));
    }
    void pullTime(std::function<FHT::iTask::state(void)> func) {
        const std::lock_guard<decltype(mutex)> lock(mutex);
        queue_.push(std::make_tuple(func, 0, false, std::chrono::high_resolution_clock::now()));
    }
    void pullTime(std::function<FHT::iTask::state(void)> func, int ms) {
        const std::lock_guard<decltype(mutex)> lock(mutex);
        queue_.push(std::make_tuple(func, ms, false, std::chrono::high_resolution_clock::now()));
    }
    bool isRun() { return isRun_; }
};

std::shared_ptr<iThread> makeThread/*<iTask::listTask::MAIN>*/() {
    return std::make_shared<Thread>();
}
template <std::size_t ... I>
std::map<std::size_t, std::shared_ptr<iThread>> FHT::Task::make_factory(std::index_sequence<I ... > const &)
{
    return {
        std::pair<std::size_t, std::shared_ptr<iThread>>{ I, makeThread/*<I>*/() } ...
    };
}
namespace FHT{
    std::shared_ptr<iTask> Conrtoller::getTask() {
        auto static a = std::make_shared<Task>();
        return a;
    }
}
