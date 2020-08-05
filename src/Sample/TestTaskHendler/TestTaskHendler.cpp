/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 06.08.19
*  Copyright (C) goganoga 2019
***************************************/
#include "iController.h"
#include "LoggerStream.h"
#include <iostream>
#include <utility>
#include <functional>

int main(void)
{
    auto H = FHT::iConrtoller::hendlerManager;
    auto T = FHT::iConrtoller::taskManager;
    auto L = FHT::iConrtoller::logger;
    L->setLevelVerboseLogging(FHT::iLogger::Verbose::INFO);

    std::function<void(int)> a([](int a) {FHT::LoggerStream::Log(FHT::LoggerStream::INFO) << a; });
    a(1);
    auto b = std::bind(a, 200000);
    b();

    H->addHendler("a", []() {FHT::LoggerStream::Log(FHT::LoggerStream::INFO) << "a"; });
    if (auto i = H->getHendler("a"); i) {
        (*i)();
    }

    auto f([](FHT::iHendler::dataRequest& data) {
        FHT::LoggerStream::Log(FHT::LoggerStream::INFO) << "b";
        FHT::LoggerStream::Log(FHT::LoggerStream::INFO) << data.portClient;
        FHT::LoggerStream::Log(FHT::LoggerStream::INFO) << data.uri;
        FHT::LoggerStream::Log(FHT::LoggerStream::INFO) << data.nextLocation;
        FHT::LoggerStream::Log(FHT::LoggerStream::INFO) << data.ipClient;
        return FHT::iHendler::dataResponse{}; });

    H->addUniqueHendler("b", f);
    FHT::iHendler::dataRequest c;
    c.portClient = 12;
    c.uri = "dasds";
    c.nextLocation = "dasdasds";
    c.ipClient = "dsdasds";
    if (auto i = H->getUniqueHendler("b"); i) {
        (*i)(c);
    }

    T->addTaskOneRun(FHT::iTask::MAIN, []() {FHT::LoggerStream::Log(FHT::LoggerStream::INFO) << "test main loop 1s is one"; }, 1000);
    T->addTaskOneRun(FHT::iTask::MAIN, b, 1);
    T->addTaskOneRun(FHT::iTask::UI, []() {FHT::LoggerStream::Log(FHT::LoggerStream::INFO) << "test loop loop 10s is one"; }, 10000);
    T->addTaskOneRun(FHT::iTask::IO, []() {FHT::LoggerStream::Log(FHT::LoggerStream::INFO) << "test io loop 7s is one"; }, 7000);

    T->addTask(FHT::iTask::UI, []() {
        FHT::LoggerStream::Log(FHT::LoggerStream::INFO) << "test main loop"; 
        return FHT::iTask::state::CONTINUE;
    });
    T->addTask(FHT::iTask::MAIN, []() {
        FHT::LoggerStream::Log(FHT::LoggerStream::INFO) << "test main loop 5s"; 
        return FHT::iTask::state::CONTINUE;
    }, 5000);
    T->addTask(FHT::iTask::MAIN, []() {
        FHT::LoggerStream::Log(FHT::LoggerStream::INFO) << "test main loop 0.5s"; 
        return FHT::iTask::state::CONTINUE;
    }, 500);
    T->addTask(FHT::iTask::IO, []() {
        FHT::LoggerStream::Log(FHT::LoggerStream::INFO) << "test io loop 0.7s"; 
        return FHT::iTask::state::CONTINUE;
    }, 700);

    std::getchar();
    return 0;

}
