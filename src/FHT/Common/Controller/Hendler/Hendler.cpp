/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 06.08.19
*  Copyright (C) goganoga 2019
***************************************/
#include "FHT/Common/Controller/Hendler/Hendler.h"
#include "FHT/LoggerStream.h"

#include <memory>
#include <functional>

namespace FHT{
    Hendler::Hendler():
        mapHendler_ptr(std::make_shared<decltype(mapHendler_)>(mapHendler_)),
        mapList_ptr(std::make_shared<decltype(mapList_)>(mapList_)),
        state_task_(iTask::state::CONTINUE){
        T->postLoopTask(iTask::FHT_MAIN, [&]() mutable {
            decltype(mapHendler_ptr) new_mapHendler_ptr;
            decltype(mapList_ptr) new_mapList_ptr;
            if (*mapHendler_ptr != mapHendler_) {
                new_mapHendler_ptr = std::make_shared<decltype(mapHendler_)>(mapHendler_);
            }
            if (*mapList_ptr != mapList_) {
                new_mapList_ptr = std::make_shared<decltype(mapList_)>(mapList_);
            }
            {
                std::lock_guard<decltype(mutex_task_)> lock(mutex_task_);
                if (new_mapHendler_ptr) mapHendler_ptr.swap(new_mapHendler_ptr);
                if (new_mapList_ptr) mapList_ptr.swap(new_mapList_ptr);
            }
            return state_task_;
        });
    }
    Hendler::~Hendler(){
        state_task_ = iTask::state::FINISH;
    }
    void Hendler::addUniqueHendler(std::string id, uniqueHendler func){
        auto mapHendler = mapHendler_ptr;
        FHT::LoggerStream::Log(FHT::LoggerStream::DEBUG) << METHOD_NAME << id;
        if (auto a = mapHendler->find(id); a != end(*mapHendler)) { 
            FHT::LoggerStream::Log(FHT::LoggerStream::WARN) << METHOD_NAME << "Found" << id;
            return;
        }
        auto prt = std::make_shared<decltype(func)>(std::move(func));
        T->postTask(iTask::FHT_MAIN, [&, id, prt]() {
            mapHendler_.emplace(id, prt);
        });
    };
    bool Hendler::removeUniqueHendler(std::string id){
        auto mapHendler = mapHendler_ptr;
        FHT::LoggerStream::Log(FHT::LoggerStream::DEBUG) << METHOD_NAME << "Found" << id;
        if (auto a = mapHendler->find(id); a != end(*mapHendler)) {
            T->postTask(iTask::FHT_MAIN, [&, id]() {
                mapHendler_.erase(id);
            });
            return true;
        }
        FHT::LoggerStream::Log(FHT::LoggerStream::DEBUG) << METHOD_NAME << "Not found" << id;
        return false;
    };
    std::shared_ptr<iHendler::uniqueHendler> Hendler::getUniqueHendler(std::string id) {
        auto mapHendler = mapHendler_ptr;
        FHT::LoggerStream::Log(FHT::LoggerStream::DEBUG) << METHOD_NAME << "Found" << id;
        if (auto a = mapHendler->find(id); a != end(*mapHendler))
            return a->second;
        FHT::LoggerStream::Log(FHT::LoggerStream::DEBUG) << METHOD_NAME << "Not found" << id;
        return {};
    };
    void Hendler::addHendler(std::string id, std::function<void(void)> func){
        auto mapList = mapList_ptr;
        FHT::LoggerStream::Log(FHT::LoggerStream::DEBUG) << METHOD_NAME << "Found" << id;
        if (auto b = mapList->find(id); b != end(*mapList)) { 
            FHT::LoggerStream::Log(FHT::LoggerStream::WARN) << METHOD_NAME << "Found" << id;
            return;
        }
        auto prt = std::make_shared<decltype(func)>(std::move(func));
        T->postTask(iTask::FHT_MAIN, [&, id, prt]() {
            mapList_.emplace(id, prt);
        });
    };
    bool Hendler::removeHendler(std::string id){
        auto mapList = mapList_ptr;
        FHT::LoggerStream::Log(FHT::LoggerStream::DEBUG) << METHOD_NAME << "Found" << id;
        if (auto a = mapList->find(id); a != end(*mapList)) {
            T->postTask(iTask::FHT_MAIN, [&, id]() {
                mapList_.erase(id);
            });
            return true;
        }
        FHT::LoggerStream::Log(FHT::LoggerStream::DEBUG) << METHOD_NAME << "Not found" << id;
        return false;
    };
    std::shared_ptr<std::function<void(void)>> Hendler::getHendler(std::string id){
        auto mapList = mapList_ptr;
        FHT::LoggerStream::Log(FHT::LoggerStream::DEBUG) << METHOD_NAME << id;
        if (auto a = mapList->find(id); a != end(*mapList))
            return a->second;
        FHT::LoggerStream::Log(FHT::LoggerStream::DEBUG) << METHOD_NAME << "Not found"  << id;
        return {};
    };
}
namespace FHT{
    std::shared_ptr<iHendler> Conrtoller::getHendler() {
        auto static a = std::make_shared<Hendler>();
        return a;
    }
}
