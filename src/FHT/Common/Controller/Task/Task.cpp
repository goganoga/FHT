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
    Task::Task() {
        isRun = startManager();
    }

    Task::~Task(){
        stopManager();
    }

    void Task::postTask(iTask::listTask thread, std::function<void(void)> func, int ms) {
        postLoopTask(thread, std::move([func = std::move(func)]() {func(); return FHT::iTask::state::FINISH; }), ms);
    }

    void Task::postLoopTask(iTask::listTask thread, std::function<state(void)> func, int ms) {
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
    std::chrono::milliseconds m_sleep;
    Threader m_internalTask;
public:
    virtual ~Thread() {
        isRun_ = false;
        thread_.reset();
    }
    Thread(): m_sleep(0) {
        thread_.reset(new std::thread{ &Thread::loop, this });
    }
    void newInternalTask(){
        auto tr = [&] {
            auto sleep = m_sleep;
            m_sleep = std::chrono::milliseconds(0);
            std::this_thread::sleep_for(sleep);
            condition_.notify_one();
        };
        Threader thread{ new std::thread(tr), [](std::thread* t) { t->join(); delete t; } };
        m_internalTask.swap(thread);
    }
    bool wait() {
        bool sleep = m_sleep.count() > 0;
        if (sleep) {
            newInternalTask();
        }
        return sleep;
    }
    size_t sizeTask() {
        std::unique_lock<std::mutex> lock_(mutex);
        for (; queue_.empty() || wait();) {
            condition_.wait(lock_);
        }
        return queue_.size();
    };
    tuple_ getTask() {
        std::lock_guard<std::mutex> lock_(mutex);
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
                        if (m_sleep.count() == 0 || m_sleep.count() > timerun) {
                            m_sleep = std::chrono::milliseconds(timerun);
                        }
                        auto difftime(std::chrono::duration_cast<std::chrono::milliseconds>(realtime - timestamp).count());
                        if (difftime < timerun) {
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
        }
    }
    void push(std::function<FHT::iTask::state(void)> func, int ms) {
        putTask(std::make_shared<Tuple>(std::move(func), ms, true, std::chrono::high_resolution_clock::now()));
    }
    bool isRun() { return isRun_; }
};

namespace FHT {
    std::shared_ptr<iThread> Task::makeThread() {
        return std::make_shared<Thread>();
    }

    template <std::size_t ... I>
    std::map<std::size_t, std::shared_ptr<iThread>> Task::make_factory(std::index_sequence<I ... > const&) {
        return {
            std::pair<std::size_t, std::shared_ptr<iThread>>{ I, makeThread() } ...
        };
    }

    std::shared_ptr<iTask> Conrtoller::getTask() {
        auto static a = std::make_shared<Task>();
        return a;
    }
}
