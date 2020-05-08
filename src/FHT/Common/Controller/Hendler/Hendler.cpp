/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 06.08.19
*  Copyright (C) goganoga 2019
***************************************/
#include "Hendler.h"
#include "Controller/Controller.h"
#include "LoggerStream.h"
#include <memory>
#include <functional>

namespace FHT{
    Hendler::Hendler(){}
    Hendler::~Hendler(){}
    void Hendler::addUniqueHendler(std::string id, uniqueHendler func){
        const std::lock_guard<decltype(mutex1)> lock(mutex1);
        FHT::LoggerStream::Log(FHT::LoggerStream::DEBUG) << METHOD_NAME << id;
        if (auto a = mapHendler_.find(id); a != end(mapHendler_)) { 
            FHT::LoggerStream::Log(FHT::LoggerStream::WARN) << METHOD_NAME << "Not found"  << id;
            throw;
        }
        mapHendler_.emplace(id, std::make_shared<decltype(func)>(std::move(func)));
    };
    bool Hendler::removeUniqueHendler(std::string id){
        const std::lock_guard<decltype(mutex1)> lock(mutex1);
        FHT::LoggerStream::Log(FHT::LoggerStream::DEBUG) << METHOD_NAME << id;
        if (auto a = mapHendler_.find(id); a != end(mapHendler_)) {
            mapHendler_.erase(a);
            return true;
        }
        FHT::LoggerStream::Log(FHT::LoggerStream::DEBUG) << METHOD_NAME << "Not found"  << id;
        return false;
    };
    std::shared_ptr<iHendler::uniqueHendler> Hendler::getUniqueHendler(std::string id) {
        const std::lock_guard<decltype(mutex1)> lock(mutex1);
        FHT::LoggerStream::Log(FHT::LoggerStream::DEBUG) << METHOD_NAME << id;
        if (auto a = mapHendler_.find(id); a != end(mapHendler_))
            return a->second;
        FHT::LoggerStream::Log(FHT::LoggerStream::DEBUG) << METHOD_NAME << "Not found"  << id;
        return emptyU_;

    };
    void Hendler::addHendler(std::string id, std::function<void(void)> func){
        const std::lock_guard<decltype(mutex2)> lock(mutex2);
        FHT::LoggerStream::Log(FHT::LoggerStream::DEBUG) << METHOD_NAME << id;
        if (auto b = mapList_.find(id); b != end(mapList_)) { 
            FHT::LoggerStream::Log(FHT::LoggerStream::WARN) << METHOD_NAME << "Not found"  << id;
            throw;
        }
        mapList_.emplace(id, std::make_shared<decltype(func)>(std::move(func)));
    };
    bool Hendler::removeHendler(std::string id){
        const std::lock_guard<decltype(mutex2)> lock(mutex2);
        FHT::LoggerStream::Log(FHT::LoggerStream::DEBUG) << METHOD_NAME << id;
        if (auto a = mapList_.find(id); a != end(mapList_)) {
            mapList_.erase(a);
            return true;
        }
        FHT::LoggerStream::Log(FHT::LoggerStream::DEBUG) << METHOD_NAME << "Not found"  << id;
        return false;

    };
    std::shared_ptr<std::function<void(void)>> Hendler::getHendler(std::string id){
        const std::lock_guard<decltype(mutex2)> lock(mutex2);
        FHT::LoggerStream::Log(FHT::LoggerStream::DEBUG) << METHOD_NAME << id;
        if (auto a = mapList_.find(id); a != end(mapList_))
            return a->second;
        FHT::LoggerStream::Log(FHT::LoggerStream::DEBUG) << METHOD_NAME << "Not found"  << id;
        return emptyV_;
    };
}
namespace FHT{
    std::shared_ptr<iHendler> Conrtoller::getHendler() {
        auto static a = std::make_shared<Hendler>();
        return a;
    }
}
