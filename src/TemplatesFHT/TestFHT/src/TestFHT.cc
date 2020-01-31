/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 06.08.19
*  Copyright (C) goganoga 2019
***************************************/
#include "iController.h"
#include <iostream>
#include <utility>
#include <functional>
int main(void)
{
    auto H = FHT::iConrtoller::hendlerManager;
    auto T = FHT::iConrtoller::taskManager;

    std::function<void(int)> a([](int a) {std::cout << a << std::endl; });
    a(1);
    auto b = std::bind(a, 200000);
    b();

    H->addHendler("a", []() {std::cout << "a" << std::endl; });
    H->getHendler("a")();

    auto f([](FHT::iHendler::data& data) {
        std::cout << "b" << std::endl;
        std::cout << data.id << std::endl;
        std::cout << data.num << std::endl;
        std::cout << data.str0 << std::endl;
        std::cout << data.str1 << std::endl;
        std::cout << data.str2 << std::endl; 
        return std::string();});
    H->addUniqueHendler("b", f);
    FHT::iHendler::data c = {12, "dasds", "dasdasds", "dsdasds", "dsds", 3213.2121};
    H->getUniqueHendler("b")(c);

    T->addTaskOneRun(FHT::iTask::MAIN, []() {std::cout << "test main loop 1s is one" << std::endl; }, 1000);
    T->addTaskOneRun(FHT::iTask::MAIN, b, 1);
    T->addTaskOneRun(FHT::iTask::UI, []() {std::cout << "test loop loop 10s is one" << std::endl; }, 10000);
    T->addTaskOneRun(FHT::iTask::IO, []() {std::cout << "test io loop 7s is one" << std::endl; }, 7000);

    T->addTask(FHT::iTask::UI, []() {
        std::cout << "test main loop" << std::endl; 
        return FHT::iTask::state::CONTINUE;
    });
    T->addTask(FHT::iTask::MAIN, []() {
        std::cout << "test main loop 5s" << std::endl; 
        return FHT::iTask::state::CONTINUE;
    }, 5000);
    T->addTask(FHT::iTask::MAIN, []() {
        std::cout << "test main loop 0.5s" << std::endl; 
        return FHT::iTask::state::CONTINUE;
    }, 500);
    T->addTask(FHT::iTask::IO, []() {
        std::cout << "test io loop 0.7s" << std::endl; 
        return FHT::iTask::state::CONTINUE;
    }, 700);

    std::getchar();
    return 0;

}
