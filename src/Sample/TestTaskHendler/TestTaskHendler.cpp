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
#include <thread> 
#include <chrono> 
std::string testHttp() {
    FHT::iClient::httpClient dd;
    dd.url = "https://yandex.ru/suggest/suggest-ya.cgi?srv=morda_ru_desktop&wiz=TrWth&uil=ru&sn=5&svg=1&part=fg&pos=2";
    auto a = dd.fetch();
    return a.body;
}

int main(void)
{
    auto H = FHT::iConrtoller::hendlerManager;
    auto T = FHT::iConrtoller::taskManager;
    auto L = FHT::iConrtoller::logger;
    L->setLevelVerboseLogging(FHT::iLogger::Verbose::INFO);

    if (!testHttp().empty()) {
        FHT::LoggerStream::Log(FHT::LoggerStream::FATAL) << "testHttp";
    }

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
