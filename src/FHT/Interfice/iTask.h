/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 06.08.19
*  Copyright (C) goganoga 2019
***************************************/
#ifndef FHTITASK_H
#define FHTITASK_H
#include <functional>
namespace FHT{
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
    };
}
#endif //FHTITASK_H
