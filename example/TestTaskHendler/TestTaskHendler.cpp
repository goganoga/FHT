/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 06.08.19
*  Copyright (C) goganoga 2019
***************************************/
#include "FHT/CoreFHT.h"
#include <iostream>
#include <utility>
#include <functional>

int main(void) {
    auto H = CoreFHT::HendlerManager;
    auto T = CoreFHT::TaskManager;

    std::function<void(int)> a([](int a) {FHT::LoggerStream::Log(FHT::LoggerStream::INFO) << a; });
    a(1);
    auto b = std::bind(a, 200000);
    b();

    T->postTask(FHT::iTask::FHT_MAIN, [=]() {
        H->addHendler("a", []() {FHT::LoggerStream::Log(FHT::LoggerStream::INFO) << "a"; });
        T->postTask(FHT::iTask::FHT_MAIN, [=]() {
            if (auto i = H->getHendler("a"); i) {
                (*i)();
            }
        },10);
    });

    T->postTask(FHT::iTask::FHT_MAIN, [=]() {
        auto f([](FHT::iHendler::dataRequest& data) {
            FHT::LoggerStream::Log(FHT::LoggerStream::INFO) << "b";
            FHT::LoggerStream::Log(FHT::LoggerStream::INFO) << data.portClient;
            FHT::LoggerStream::Log(FHT::LoggerStream::INFO) << data.uri;
            FHT::LoggerStream::Log(FHT::LoggerStream::INFO) << data.nextLocation;
            FHT::LoggerStream::Log(FHT::LoggerStream::INFO) << data.ipClient;
            return FHT::iHendler::dataResponse{};
            });
        H->addUniqueHendler("b", f);
        T->postTask(FHT::iTask::FHT_MAIN, [=]() {
            if (auto i = H->getUniqueHendler("b"); i) {
                FHT::iHendler::dataRequest c;
                c.portClient = 12;
                c.uri = "dasds";
                c.nextLocation = "dasdasds";
                c.ipClient = "dsdasds";
                (*i)(c);
            }
        },10);
    });

    T->postTask(FHT::iTask::MAIN, []() {FHT::LoggerStream::Log(FHT::LoggerStream::INFO) << "test main loop 1s is one"; }, 1000);
    T->postTask(FHT::iTask::MAIN, b, 1);
    T->postTask(FHT::iTask::UI, []() {FHT::LoggerStream::Log(FHT::LoggerStream::INFO) << "test ui loop 10s is one"; }, 10000);
    T->postTask(FHT::iTask::IO, []() {FHT::LoggerStream::Log(FHT::LoggerStream::INFO) << "test io loop 7s is one"; }, 7000);

    T->postLoopTask(FHT::iTask::UI, []() {
        FHT::LoggerStream::Log(FHT::LoggerStream::INFO) << "test ui loop";
        return FHT::iTask::state::CONTINUE;
    });
    T->postLoopTask(FHT::iTask::MAIN, []() {
        FHT::LoggerStream::Log(FHT::LoggerStream::INFO) << "test main loop 5s"; 
        return FHT::iTask::state::CONTINUE;
    }, 5000);
    T->postLoopTask(FHT::iTask::MAIN, []() {
        FHT::LoggerStream::Log(FHT::LoggerStream::INFO) << "test main loop 0.5s";
        return FHT::iTask::state::CONTINUE;
    }, 500);
    T->postLoopTask(FHT::iTask::IO, []() {
        FHT::LoggerStream::Log(FHT::LoggerStream::INFO) << "test io loop 0.7s"; 
        return FHT::iTask::state::CONTINUE;
    }, 700);
    std::getchar();
    return 0;

}
