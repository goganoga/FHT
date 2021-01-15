/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 06.08.19
*  Copyright (C) goganoga 2019
***************************************/
#ifndef FHTTEST_H
#define FHTTEST_H
#include "../iTest.h"
#include "FHT/CoreFHT.h"
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
        FHT::iHendler::dataResponse mainTest(iHendler::dataRequest& resp);
        FHT::iHendler::dataResponse mainTestGet(iHendler::dataRequest& resp);
        FHT::iHendler::dataResponse mainTestWebSocket(iHendler::dataRequest& resp);
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
