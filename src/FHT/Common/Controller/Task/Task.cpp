/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 06.08.19
*  Copyright (C) goganoga 2019
***************************************/
#include "FHT/Common/Controller/Task/Task.h"
#include "FHT/Common/Controller/Controller.h"
#include "FHT/LoggerStream.h"

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
struct size {
    size(std::function<size_t(void)> con, std::function<void(void)> dis) : m_dis(dis) {
        m_size = con();
    }
    size_t getSize() {
        return m_size;
    }
    ~size() {
        if (m_size > 0) {
            m_dis();
        }
    }
private:
    std::function<void(void)> m_dis;
    size_t m_size;
};
class internalThread {
    std::atomic<std::chrono::milliseconds> m_sleep = std::chrono::milliseconds(-1);
    bool m_wait = false;
    std::function<void(void)> m_ready;
    bool volatile isRun_ = true;
    std::mutex m_mx;
public:
    internalThread(std::function<void(void)> ready) :m_ready(ready) {}
    virtual ~internalThread() {
        isRun_ = false;
    }
    std::chrono::milliseconds getSleep() {
        return m_sleep.load();
    }
    void setSleep(std::chrono::milliseconds sleep) {
        m_sleep.store(sleep);
    }
    bool isWait() {
        std::lock_guard<std::mutex> lock_(m_mx);
        m_wait = getSleep().count() > 0;
        return m_wait;
    }
    void loop() {
        while (isRun_) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            std::unique_lock<std::mutex> lock_(m_mx);
            if (m_wait) {
                lock_.unlock();
                m_wait = !m_wait;
                std::this_thread::sleep_for(getSleep());
                m_ready();
                setSleep(std::chrono::milliseconds(-1));
            }
        }
    }
};
class Thread : public iThread {
    std::mutex mutex;
    std::condition_variable condition_;
    using Threader = std::unique_ptr<std::thread, std::function<void(std::thread *)>>;
    Threader thread_{ nullptr, [](std::thread *a) { if (a) a->join(); delete a; } };
    bool volatile isRun_ = true;
    using tuple_ = std::shared_ptr<Tuple>;
    std::queue<tuple_> queue_;
    internalThread m_internalThread;
    Threader m_internalTask{ nullptr, [](std::thread* a) { if (a) a->join(); delete a; } };
public:
    virtual ~Thread() {
        isRun_ = false;
        thread_.reset();
        m_internalTask.reset();
    }
    Thread() :m_internalThread([this] { putTask({}); }) {
        thread_.reset(new std::thread{ &Thread::loop, this });
        m_internalTask.reset(new std::thread{ &internalThread::loop, &m_internalThread });
    }
    bool wait() {
        return m_internalThread.isWait();
    }
    size sizeTask() {
        return { 
            [&] {
                std::unique_lock<std::mutex> lock_(mutex);
                for (; queue_.empty() ;) {
                    condition_.wait(lock_);
                }
                return queue_.size();
            }, 
            [&] {
                std::unique_lock<std::mutex> lock_(mutex);
                for (; wait();) {
                    condition_.wait(lock_);
                }
            } 
        };
    };
    tuple_ getTask() {
        std::unique_lock<std::mutex> lock_(mutex);
        auto task = queue_.front();
        queue_.pop();
        lock_.unlock();
        return task;
    };
    void putTask(tuple_ task) {
        std::unique_lock<std::mutex> lock_(mutex);
        if (task) {
            queue_.push(task);
        }
        lock_.unlock();
        condition_.notify_one();
    };
    void loop() {
        while (isRun_) {
            try {
                long long sleep = 0;
                auto realtime = std::chrono::high_resolution_clock::now();
                auto size_task = sizeTask();
                for (size_t size = size_task.getSize(); size > 0; --size) {
                    tuple_ task = getTask();
                    if (task) {
                        auto& timestamp = task->ts;
                        auto& timerun = task->ms;
                        if (sleep > timerun) {
                            sleep = timerun;
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
                if (sleep == 0) {
                    sleep = 1;
                }
                m_internalThread.setSleep(std::chrono::milliseconds(sleep));
            } catch (std::exception const& e) {
                FHT::LoggerStream::Log(FHT::LoggerStream::ERR) << METHOD_NAME << e.what();
            }
        }
    }
    void push(std::function<FHT::iTask::state(void)> func, int ms) {
        m_internalThread.setSleep(std::chrono::milliseconds(-1));
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
