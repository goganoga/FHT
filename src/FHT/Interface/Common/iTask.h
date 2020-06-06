/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 06.08.19
*  Copyright (C) goganoga 2019
***************************************/
#ifndef FHTITASK_H
#define FHTITASK_H
#include <functional>
#include <chrono>
namespace FHT{
    struct iTask {
        virtual ~iTask() = default;
        enum listTask {
            FHT_MAIN,
            MAIN,
            MATH,
            IO,
            UI,
            size
        };
        enum struct state {
            FINISH,
            CONTINUE
        };
        virtual void addTask(iTask::listTask thread, std::function<state(void)> func) = 0;
        virtual void addTask(iTask::listTask thread, std::function<state(void)> func, int ms) = 0;
        virtual void addTaskOneRun(iTask::listTask thread, std::function<void(void)> func) = 0;
        virtual void addTaskOneRun(iTask::listTask thread, std::function<void(void)> func, int ms) = 0;
        virtual void setDeltaTime(std::chrono::microseconds delta_time) = 0;
    };
}
#endif //FHTITASK_H
