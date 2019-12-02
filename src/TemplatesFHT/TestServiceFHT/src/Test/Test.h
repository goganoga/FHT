/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 06.08.19
*  Copyright (C) goganoga 2019
***************************************/
#ifndef FHTTEST_H
#define FHTTEST_H
#include "../iTest.h"
#include "iController.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <cstdint>
#include <vector>
#include <sstream>
#include <memory>
#include <functional>
#include <map>
namespace FHT {
    class Test : public iTest {
        std::string mainTest(iHendler::data& resp);
        std::string mainTestGet(iHendler::data& resp);
        std::string md5Hash(const char* string);
        std::string guid();
        std::string guid(std::string ab);
        std::string hash(const char *string);
        std::string hash512(const char *string);
        std::string gen();
        std::string jsonParse(std::map<std::string, std::string> map);
    public:
        Test();
        virtual ~Test() override {};
    };
}
#endif //FHTTEST_H
