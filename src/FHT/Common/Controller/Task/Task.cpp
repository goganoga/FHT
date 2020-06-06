/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 06.08.19
*  Copyright (C) goganoga 2019
***************************************/
#include "Task.h"
#include "Controller/Controller.h"
#include "LoggerStream.h"

#include <chrono>
#include <condition_variable>
#include <thread>
#include <mutex>
#include <utility>
#include <map>
#include <iostream>
#include <queue>
#include <tuple>
#include <memory>

namespace FHT{
    Task::Task(): delta_time_(std::chrono::microseconds(100)) {
        isRun = startManager();
    }

    Task::~Task(){
        stopManager();
    }

    void Task::addTaskOneRun(iTask::listTask thread, std::function<void(void)> func) {
        addTask(thread, std::move([func = std::move(func)]() {func(); return FHT::iTask::state::FINISH; }));
    }

    void Task::addTask(iTask::listTask thread, std::function<state(void)> func) {
        addTask(thread, func, 0);
    }

    void Task::addTaskOneRun(iTask::listTask thread, std::function<void(void)> func, int ms) {
        addTask(thread, std::move([func = std::move(func)]() {func(); return FHT::iTask::state::FINISH; }), ms);
    }

    void Task::addTask(iTask::listTask thread, std::function<state(void)> func, int ms) {
        if (isRun) {
            getTaskThread(thread)->push(std::move(func), ms);
        }
    }

    std::shared_ptr<iThread> Task::getTaskThread(iTask::listTask thread) {
        if(auto a = factory_.find(static_cast<std::size_t>(thread)); end(factory_) != a)
            return a->second;
        return nullptr;
    }

    bool Task::startManager() {
        factory_ = Task::make_factory(delta_time_, std::make_index_sequence<iTask::listTask::size>{});
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
struct Tuple {
    Tuple(std::function<FHT::iTask::state(void)> function_, long long ms_, bool isLoop_, decltype(std::chrono::high_resolution_clock::now()) ts_):
        function(std::move(function_)),
        ms(ms_),
        isLoop(isLoop_),
        ts(ts_){}
    ~Tuple() {
    }
    std::function<FHT::iTask::state(void)> function;
    long long ms;
    bool isLoop;
    decltype(std::chrono::high_resolution_clock::now()) ts;
};
class Thread : public iThread {
    std::mutex mutex;
    std::condition_variable condition_;
    using Threader = std::unique_ptr<std::thread, std::function<void(std::thread *)>>;
    Threader thread_{ nullptr, [](std::thread *a) { if (a) a->join(); delete a; } };
    bool volatile isRun_ = true;
    using tuple_ = std::shared_ptr<Tuple>;
    std::queue<tuple_> queue_;
    std::atomic<std::chrono::microseconds> &sleep_;
public:
    virtual ~Thread() {
        isRun_ = false;
        thread_.reset();
    }
    Thread(std::atomic<std::chrono::microseconds> &sleep) : sleep_(sleep) {
        thread_.reset(new std::thread{ &Thread::loop, this });
    }
    size_t sizeTask() {
        std::lock_guard<std::mutex> lock_(mutex);
        return queue_.size();
    };
    tuple_ getTask() {
        std::unique_lock<std::mutex> lock_(mutex);
        for (; queue_.empty();) {
            condition_.wait(lock_);
        }
        auto task = queue_.front();
        queue_.pop();
        return task;
    };
    void putTask(tuple_ task) {
        std::unique_lock<std::mutex> lock_(mutex);
        queue_.push(task);
        lock_.unlock();
        condition_.notify_one();
    };
    void loop() {
        while (isRun_) {
            try {
                for (size_t size = sizeTask(); size > 0; --size) {
                    tuple_ task = getTask();
                    if (task) {
                        auto realtime = std::chrono::high_resolution_clock::now();
                        auto& timestamp = task->ts;
                        auto& timerun = task->ms;
                        auto difftime(std::chrono::duration_cast<std::chrono::milliseconds>(realtime - timestamp));
                        if (difftime.count() < timerun) {
                            putTask(task);
                        }
                        else {
                            auto& functor = task->function;
                            if (functor && functor() == FHT::iTask::state::CONTINUE && task->isLoop) {
                                timestamp = std::chrono::high_resolution_clock::now();
                                putTask(task);
                            }
                        }
                    }
                    else {
                        FHT::LoggerStream::Log(FHT::LoggerStream::ERR) << METHOD_NAME;
                    }
                }
            } catch (std::exception const& e) {
                FHT::LoggerStream::Log(FHT::LoggerStream::ERR) << METHOD_NAME << e.what();
            }
            std::this_thread::sleep_for(sleep_.load());
        }
    }
    void push(std::function<FHT::iTask::state(void)> func, int ms) {
        putTask(std::make_shared<Tuple>(std::move(func), ms, true, std::chrono::high_resolution_clock::now()));
    }
    bool isRun() { return isRun_; }
};

namespace FHT {
    std::shared_ptr<iThread> Task::makeThread(std::atomic<std::chrono::microseconds> &delta_time) {
        return std::make_shared<Thread>(delta_time);
    }

    void Task::setDeltaTime(std::chrono::microseconds delta_time) {
        if (std::chrono::microseconds(1).count() < delta_time.count()) {
            delta_time_.store(delta_time);
        }
    }

    template <std::size_t ... I>
    std::map<std::size_t, std::shared_ptr<iThread>> Task::make_factory(std::atomic<std::chrono::microseconds> &delta_time, std::index_sequence<I ... > const&) {
        return {
            std::pair<std::size_t, std::shared_ptr<iThread>>{ I, makeThread(delta_time) } ...
        };
    }

    std::shared_ptr<iTask> Conrtoller::getTask() {
        auto static a = std::make_shared<Task>();
        return a;
    }
}
