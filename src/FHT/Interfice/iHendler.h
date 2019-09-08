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
#include <cstring>
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
                struct FHT_KV *next = nullptr;
                struct FHT_KV **prev = nullptr;
            };
            char *key = nullptr;
            char *value = nullptr;
        };
        struct FHT_MAP {
                struct FHT_KV *first = nullptr;
                struct FHT_KV **last = nullptr;
                char* find(char* param ) {
                    auto a = this->first;
                    while (a) {
                        if (*a->key == *param)
                            return a->value;
                        a = a->next;
                    }
                    return nullptr;
                }
                FHT_MAP() {}
                FHT_MAP(std::string str) {
                    FHT_KV *a = new FHT_KV;
                    this->first = a;
                    this->last = new FHT_KV*;
                    std::string buf;
                    for (auto b : str) {
                        if (b == '=') {
                            a->prev = new FHT_KV*;
                            *a->prev = *&a;
                            a->key = new char[buf.size()];
                            strcpy(a->key, buf.data());
                            buf.clear();
                        } else if (b == '&') {
                            a->value = new char[buf.size()];
                            strcpy(a->value, buf.data());
                            buf.clear();
                            FHT_KV *c = new FHT_KV;
                            a->next = c;
                            a = c;
                        } else {
                            buf += b;
                        }
                    }
                    a->value = new char[buf.size()];
                    strcpy(a->value, buf.data());
                    *this->last = *&a;
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
