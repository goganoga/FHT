/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 06.08.19
*  Copyright (C) goganoga 2019
***************************************/
#ifndef FHTHENDLER_H
#define FHTHENDLER_H
#include "../../../Interfice/iHendler.h"
#include <map>

namespace FHT{
    class Hendler: public iHendler
    {
    public:
        Hendler();
        void addUniqueHendler(std::string id, std::function<std::string(iHendler::data)> func) override final;
        void addHendler(std::string id, std::function<void(void)> func) override final;
        bool removeUniqueHendler(std::string id) override final;
        bool removeHendler(std::string id) override final;
        std::function<std::string(iHendler::data)> getUniqueHendler(std::string id) override final;
        std::function<void(void)> getHendler(std::string id) override final;
        virtual ~Hendler() override;
    private:
        std::map<std::string, std::function<std::string(iHendler::data)>> mapHendler;
        std::map<std::string, std::function<void(void)>> mapList;
    };
}
#endif //FHTHENDLER_H
