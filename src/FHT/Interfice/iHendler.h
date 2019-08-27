/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 06.08.19
*  Copyright (C) goganoga 2019
***************************************/
#ifndef FHTIHENDLER_H
#define FHTIHENDLER_H
#include <functional>
#include <string>
#include <any>
namespace FHT{
    struct iHendler {
        virtual ~iHendler() = default;
        struct data {
            int id = 0;
            std::string str0;
            std::string str1;
            std::string str2;
            std::string str3;
            double num = 0;
            std::any obj1;
            void* obj2 = nullptr;

        };
		struct FHT_KV {
			struct {
				struct FHT_KV *next;
				struct FHT_KV **prev;
			};
			char *key;
			char *value;
		};
		struct FHT_MAP {
				struct FHT_KV *first;
				struct FHT_KV **last;
				char* find(char* param ) {
					auto a = this->first;
					while (a) {
						if (*a->key == *param)
							return a->value;
						a = a->next;
					}
					return nullptr;
				}
		};
        virtual void addUniqueHendler(std::string id, std::function<std::string(iHendler::data)> func) = 0;
        virtual bool removeUniqueHendler(std::string id) = 0;
        virtual std::function<std::string(iHendler::data)> getUniqueHendler(std::string id) = 0;

        virtual void addHendler(std::string id, std::function<void(void)> func) = 0;
        virtual bool removeHendler(std::string id) = 0;
        virtual std::function<void(void)> getHendler(std::string id) = 0;
    };
}
#endif //FHTIHENDLER_H
