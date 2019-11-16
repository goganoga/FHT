/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 06.08.19
*  Copyright (C) goganoga 2019
***************************************/
#include "Task.h"
#include "../Controller.h"
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
public:
    virtual ~Thread() {
        isRun_ = false;
        thread_.reset();
    }
	Thread() {
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
            auto size = queue_.size();
            auto sleep = std::chrono::nanoseconds(100);
            if(size < 10) sleep = std::chrono::microseconds(100); else
            if(size < 100) sleep = std::chrono::microseconds(10); else
            if(size < 1000) sleep = std::chrono::microseconds(1);
            std::lock_guard<std::mutex> lock(mutex);
            std::this_thread::sleep_for(sleep);
            tuple_ a;
            try{
                if(!queue_.empty())
                    a = queue_.front();
                else
                    continue;
            }
            catch(std::exception e) {
                continue;
            }
            queue_.pop();
            auto realtime = std::chrono::high_resolution_clock::now();
            auto timestamp = std::get<tuple::ts>(a);
            auto timerun = std::get<tuple::ms>(a);
            auto difftime(std::chrono::duration_cast<std::chrono::milliseconds>(realtime - timestamp));
            if(difftime.count() < timerun){
                queue_.push(a);
            } else {
				FHT::iTask::state result = std::get<tuple::function>(a)();
                if(result == FHT::iTask::state::CONTINUE && std::get<tuple::isLoop>(a)){
                    std::get<tuple::ts>(a) = std::chrono::high_resolution_clock::now();
                    queue_.push(a);
                }
            }
        }
	}
	void pull(std::function<FHT::iTask::state(void)> func) {
        //std::lock_guard<std::mutex> lock(mutex);
        queue_.push(std::make_tuple(func, 0, true, std::chrono::high_resolution_clock::now()));
    }
	void pull(std::function<FHT::iTask::state(void)> func, int ms) {
        //std::lock_guard<std::mutex> lock(mutex);
        queue_.push(std::make_tuple(func, ms, true, std::chrono::high_resolution_clock::now()));
    }
	void pullTime(std::function<FHT::iTask::state(void)> func) {
		//std::lock_guard<std::mutex> lock(mutex);
		queue_.push(std::make_tuple(func, 0, false, std::chrono::high_resolution_clock::now()));
	}
    void pullTime(std::function<FHT::iTask::state(void)> func, int ms) {
        //std::lock_guard<std::mutex> lock(mutex);
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
