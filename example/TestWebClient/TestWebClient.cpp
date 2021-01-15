/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 17.01.21
*  Copyright (C) goganoga 2021
***************************************/
#include "FHT/CoreFHT.h"

bool testHttps() {
    FHT::iClient::httpClient dd;
    dd.url = "https://yandex.ru/suggest/suggest-ya.cgi?srv=morda_ru_desktop&wiz=TrWth&uil=ru&sn=5&svg=1&part=fg&pos=2";
    auto a = dd.fetch();
    std::string result = a.getStringFromBody();
    FHT::LoggerStream::Log(FHT::LoggerStream::INFO) << result;
    return !result.empty();
}
bool testHttp() {
    FHT::iClient::httpClient dd;
    dd.url = "http://localhost:10800/test?qq=test";
    dd.setStringToBody("test-body");
    auto a = dd.fetch();
    std::string result = a.getStringFromBody();
    FHT::LoggerStream::Log(FHT::LoggerStream::INFO) << result;
    return !result.empty();
}
int main(void) {
    if (!testHttp() || !testHttps()) {
        FHT::LoggerStream::Log(FHT::LoggerStream::FATAL) << "testHttp";
    }

    std::getchar();
    return 0;

}
