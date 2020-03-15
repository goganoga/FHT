/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 06.08.19
*  Copyright (C) goganoga 2019
***************************************/
#include "Hendler.h"
#include "Controller/Controller.h"
#include <memory>
#include <functional>

namespace FHT{
    Hendler::Hendler(){}
    Hendler::~Hendler(){}
    void Hendler::addUniqueHendler(std::string id, uniqueHendler func){
        const std::lock_guard<decltype(mutex1)> lock(mutex1);
        if (auto a = mapHendler_.find(id); a != end(mapHendler_))
            throw;
        if (auto b = mapList_.find(id); b != end(mapList_))
            throw;
        mapHendler_.emplace(id, func);
    };
    bool Hendler::removeUniqueHendler(std::string id){
        const std::lock_guard<decltype(mutex1)> lock(mutex1);
        if (auto a = mapHendler_.find(id); a != end(mapHendler_)) {
            mapHendler_.erase(a);
            return true;
        }
        return false;
    };
    Hendler::uniqueHendler& Hendler::getUniqueHendler(std::string id) {
        const std::lock_guard<decltype(mutex1)> lock(mutex1);
        if (auto a = mapHendler_.find(id); a != end(mapHendler_))
            return a->second;
        return emptyU_;

    };
    void Hendler::addHendler(std::string id, std::function<void(void)> func){
        const std::lock_guard<decltype(mutex2)> lock(mutex2);
        if (auto a = mapHendler_.find(id); a != end(mapHendler_))
            throw;
        if (auto b = mapList_.find(id); b != end(mapList_))
            throw;
        mapList_.emplace(id, func);
    };
    bool Hendler::removeHendler(std::string id){
        const std::lock_guard<decltype(mutex2)> lock(mutex2);
        if (auto a = mapList_.find(id); a != end(mapList_)) {
            mapList_.erase(a);
            return true;
        }
        return false;

    };
    std::function<void(void)>& Hendler::getHendler(std::string id){
        const std::lock_guard<decltype(mutex2)> lock(mutex2);
        if (auto a = mapList_.find(id); a != end(mapList_))
            return a->second;
        return emptyV_;
    };
}
namespace FHT{
    std::shared_ptr<iHendler> Conrtoller::getHendler() {
        auto static a = std::make_shared<Hendler>();
        return a;
    }
}
