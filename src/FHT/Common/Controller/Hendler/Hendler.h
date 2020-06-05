/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 06.08.19
*  Copyright (C) goganoga 2019
***************************************/
#ifndef FHTHENDLER_H
#define FHTHENDLER_H
#include "Common/iHendler.h"
#include <map>
#include <mutex>

namespace FHT {
    class Hendler: public iHendler {
    public:
        Hendler();
        void addUniqueHendler(std::string id, uniqueHendler func) override final;
        void addHendler(std::string id, std::function<void(void)> func) override final;
        bool removeUniqueHendler(std::string id) override final;
        bool removeHendler(std::string id) override final;
        std::shared_ptr<uniqueHendler> getUniqueHendler(std::string id) override final;
        std::shared_ptr<std::function<void(void)>> getHendler(std::string id) override final;
        virtual ~Hendler() override;
    private:
        std::map<std::string, std::shared_ptr<uniqueHendler>> mapHendler_;
        std::map<std::string, std::shared_ptr<std::function<void(void)>>> mapList_;

        std::mutex mutex1;
        std::mutex mutex2;
    };
}
#endif //FHTHENDLER_H
