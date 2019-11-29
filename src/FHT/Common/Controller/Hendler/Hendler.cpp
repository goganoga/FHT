/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 06.08.19
*  Copyright (C) goganoga 2019
***************************************/
#include "Hendler.h"
#include "Controller/Controller.h"
#include <memory>
namespace FHT{
    Hendler::Hendler(){}
    Hendler::~Hendler(){}
    void Hendler::addUniqueHendler(std::string id, uniqueHendler func){
        if (auto a = mapHendler.find(id); a != end(mapHendler))
            throw;
        if (auto b = mapList.find(id); b != end(mapList))
            throw;
        mapHendler.emplace(id, func);
    };
    bool Hendler::removeUniqueHendler(std::string id){
        if (auto a = mapHendler.find(id); a != end(mapHendler)) {
            mapHendler.erase(a);
            return true;
        }
        return false;
    };
	Hendler::uniqueHendler& Hendler::getUniqueHendler(std::string id) {
        if (auto a = mapHendler.find(id); a != end(mapHendler))
            return a->second;
		return std::forward<uniqueHendler>(nullptr);

    };
    void Hendler::addHendler(std::string id, std::function<void(void)> func){
        if (auto a = mapHendler.find(id); a != end(mapHendler))
            throw;
        if (auto b = mapList.find(id); b != end(mapList))
            throw;
        mapList.emplace(id, func);
    };
    bool Hendler::removeHendler(std::string id){
        if (auto a = mapList.find(id); a != end(mapList)) {
            mapList.erase(a);
            return true;
        }
        return false;

    };
    std::function<void(void)>& Hendler::getHendler(std::string id){
        if (auto a = mapList.find(id); a != end(mapList))
            return a->second;
		return std::forward<std::function<void(void)>>(nullptr);
    };
}
namespace FHT{
	std::shared_ptr<iHendler> Conrtoller::getHendler() {
		auto static a = std::make_shared<Hendler>();
		return a;
	}
}
