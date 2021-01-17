/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 06.08.19
*  Copyright (C) goganoga 2019
***************************************/
#include "Test.h"
#include "FHT/LoggerStream.h"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/name_generator.hpp>
#include <boost/uuid/nil_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <openssl/sha.h>
#include <openssl/md5.h>

namespace FHT {
    std::shared_ptr<iTest> iTest::Run = std::make_shared<Test>();
    Test::Test() {
        auto H = CoreFHT::HendlerManager;
        H->addUniqueHendler("/test", [this](FHT::iHendler::dataRequest &data){return mainTest(data);});
        H->addUniqueHendler("/testGet", [this](FHT::iHendler::dataRequest &data){return mainTestGet(data);});
        H->addUniqueHendler(FHT::webSocket("/testWS"), [this](FHT::iHendler::dataRequest &data){return mainTestWebSocket(data);});
    }
    FHT::iHendler::dataResponse Test::mainTest(FHT::iHendler::dataRequest& resp) {
        std::string buf = resp.uri;
        std::string location = resp.nextLocation;
        auto headers = resp.params;
        std::string postBody(resp.getStringFromBody());

        FHT::iHendler::dataResponse body;
        try {
            std::map< std::string, std::string> resp_map;
            auto postParam(postBody);
            std::string param_test("null");
            auto test = headers.find("qq");
            if (test != headers.end())
                param_test = test->second;
            std::string hash_buf = buf;
            hash_buf.append(md5Hash(buf.c_str()));
            hash_buf.append(std::to_string(time(nullptr)));
            hash_buf = hash(hash_buf.c_str());

            resp_map.emplace("status", "1");
            resp_map.emplace("uri", buf);
            resp_map.emplace("location", location);
            resp_map.emplace("postBody", postBody);
            resp_map.emplace("param[qq]", param_test);
            resp_map.emplace("hash_sha256", hash(buf.c_str()));
            resp_map.emplace("hash_sha256", hash(buf.c_str()));
            resp_map.emplace("hash_md5", md5Hash(buf.c_str()));
            resp_map.emplace("guid", guid());
            resp_map.emplace("guid_location", guid(buf));
            resp_map.emplace("hash_sha512", hash512(buf.c_str()));
            resp_map.emplace("h_sha512_gen", gen());
            resp_map.emplace("hash_h", hash_buf);

            std::string str(jsonParse(resp_map));
            body.setStringToBody(str);
        }
        catch (const std::exception& e) {
            FHT::LoggerStream::Log(FHT::LoggerStream::ERR) << METHOD_NAME << e.what();
        }
        return body;
    }

    FHT::iHendler::dataResponse Test::mainTestGet(FHT::iHendler::dataRequest& resp) {
        auto headers = resp.params;
        std::map< std::string, std::string> resp_map;
        FHT::iHendler::dataResponse body;
        try {
            std::string param_test("http://localhost:10800/test?qq=test&test=test");
            auto test = headers.find("url");
            if (test != headers.end())
                param_test = test->second;
            FHT::iClient::httpClient reruest;
            reruest.url = param_test;
            auto result = reruest.fetch();
            std::string bodyRequest = result.getStringFromBody();
            resp_map.emplace("GetUrl", param_test);
            resp_map.emplace("bodyRequest", bodyRequest);
        }
        catch (const std::exception& e) {
            FHT::LoggerStream::Log(FHT::LoggerStream::ERR) << METHOD_NAME << e.what();
        }
        std::string str(jsonParse(resp_map));
        body.setStringToBody(str);

        return body;
    }

    FHT::iHendler::dataResponse Test::mainTestWebSocket(FHT::iHendler::dataRequest& resp) {
        std::weak_ptr<FHT::wsSubscriber> func = std::any_cast<std::weak_ptr<FHT::wsSubscriber>>(resp.WSInstanse);
        std::map<std::string, std::string> resp_map;
        try {
            CoreFHT::TaskManager->postLoopTask(FHT::iTask::MAIN, [func]() mutable {
                std::string str("WebSocket Test!!!");
                auto func_ptr = func.lock();
                if (func_ptr && func_ptr->getPublisher(str)) {
                    return FHT::iTask::state::CONTINUE;
                }
                return FHT::iTask::state::FINISH;
            }, 1000);
            CoreFHT::TaskManager->postLoopTask(FHT::iTask::MAIN, [func]() mutable {
                auto func_ptr = func.lock();
                if (func_ptr) {
                    func_ptr->stop();
                }
                return FHT::iTask::state::FINISH;
            }, 40000);

            func.lock()->setSubscriber([func](std::string& msg) {
                if (auto func_ptr = func.lock(); func_ptr) {
                    func_ptr->getPublisher(msg);
                }
            });
        }
        catch (const std::exception& e) {
            FHT::LoggerStream::Log(FHT::LoggerStream::ERR) << METHOD_NAME << e.what();
        }
        return FHT::iHendler::dataResponse{};
    }
    std::string Test::md5Hash(const char* string) {
        unsigned char digest[MD5_DIGEST_LENGTH];
        MD5_CTX ctx;
        MD5_Init(&ctx);
        MD5_Update(&ctx, string, strlen(string));
        MD5_Final(digest, &ctx);
        char outputBuffer[33];
        for (int i = 0; i < MD5_DIGEST_LENGTH; i++)
            sprintf(&outputBuffer[i * 2], "%02x", (unsigned int)digest[i]);
        return std::string(outputBuffer);
    }
    std::string Test::guid() {
        boost::uuids::random_generator generator;
        boost::uuids::uuid a = generator();
        std::string tmp = boost::uuids::to_string(a);
        return tmp;
    }
    std::string Test::guid(std::string ab) { //uuid_v5
        boost::uuids::nil_generator gen_nil;
        boost::uuids::uuid namespace_uuid;
        namespace_uuid = gen_nil();
        boost::uuids::name_generator gen(namespace_uuid);
        boost::uuids::uuid u = gen(ab.c_str());
        std::string tmp = boost::uuids::to_string(u);
        return tmp;
    }
    std::string Test::hash(const char *string) {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256_CTX sha256;
        SHA256_Init(&sha256);
        SHA256_Update(&sha256, string, strlen(string));
        SHA256_Final(hash, &sha256);
        char outputBuffer[SHA256_CBLOCK + 1];
        for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
            sprintf(outputBuffer + (i * 2), "%02x", hash[i]);
        return std::string(outputBuffer);
    }
    std::string Test::hash512(const char *string) {
        unsigned char hash[SHA512_DIGEST_LENGTH];
        SHA512_CTX sha512;
        SHA512_Init(&sha512);
        SHA512_Update(&sha512, string, strlen(string));
        SHA512_Final(hash, &sha512);
        char outputBuffer[SHA512_CBLOCK + 1];
        for (int i = 0; i < SHA512_DIGEST_LENGTH; i++)
            sprintf(outputBuffer + (i * 2), "%02x", hash[i]);
        return std::string(outputBuffer);
    }
    std::string Test::gen() {
        int length = rand() % 3000;
        auto randchar = []() -> char
        {
            const char charset[] =
                "0123456789"
                "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                "abcdefghijklmnopqrstuvwxyz";
            const size_t max_index = (sizeof(charset) - 1);
            return charset[rand() % max_index];
        };
        std::string str(length, 0);
        std::generate_n(str.begin(), length, randchar);
        return hash512(str.data());
    }
    std::string Test::jsonParse(std::map<std::string, std::string> map)
    {
        std::string slash = "\"";
        std::string twoPoint = ":";
        std::string comma = ",";
        std::string figureBrackedLeft = "{";
        std::string figureBrackedRight = "}";
        std::string strJson;
        strJson.append(figureBrackedLeft);
        for(auto a: map){
            strJson.append(slash + a.first + slash);
            strJson.append(twoPoint);
            strJson.append(slash + a.second + slash);
            strJson.append(comma);
        }
        if(strJson.size()>1)
            strJson.pop_back();
        strJson.append(figureBrackedRight);
        return strJson;
    }
}