/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 06.08.19
*  Copyright (C) goganoga 2019
***************************************/
#include "Test.h"
#include "LoggerStream.h"
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
        auto H = FHT::iConrtoller::hendlerManager;
        H->addUniqueHendler("/test", [this](FHT::iHendler::data &data){return mainTest(data);});
        H->addUniqueHendler("/testGet", [this](FHT::iHendler::data &data){return mainTestGet(data);});
        H->addUniqueHendler(FHT::webSocket("/testWS"), [this](FHT::iHendler::data &data){return mainTestWebSocket(data);});
    }
    std::string Test::mainTest(FHT::iHendler::data& resp) {
        std::string buf = resp.str0;
        std::string location = resp.str1;
        auto headers = resp.map0;
        std::string postBody = resp.str2;

        std::string body;
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

            body = jsonParse(resp_map);
        }
        catch (const std::exception& e) {
            FHT::LoggerStream::Log(FHT::LoggerStream::ERR) << METHOD_NAME << e.what();
        }
        return body;
    }

    std::string Test::mainTestGet(FHT::iHendler::data& resp) {
        auto headers = resp.map0;
        std::map< std::string, std::string> resp_map;
        char* id_buf = nullptr;
        try {
            std::string param_test("http://localhost:10800/test?qq=test&test=test");
            auto test = headers.find("url");
            if (test != headers.end())
                param_test = test->second;
            std::string bodyRequest = FHT::iConrtoller::webClient->get(param_test);
            resp_map.emplace("GetUrl", param_test);
            resp_map.emplace("bodyRequest", bodyRequest);
        }
        catch (const std::exception& e) {
            FHT::LoggerStream::Log(FHT::LoggerStream::ERR) << METHOD_NAME << e.what();
            delete id_buf;
        }
        delete id_buf;
        return jsonParse(resp_map);
    }

    std::string Test::mainTestWebSocket(FHT::iHendler::data& resp) {
        std::weak_ptr<FHT::wsSubscriber> func = std::any_cast<std::weak_ptr<FHT::wsSubscriber>>(resp.obj1);
        std::map<std::string, std::string> resp_map;
        try {
            FHT::iConrtoller::taskManager->addTask(FHT::iTask::MAIN, [func]() mutable {
                std::string str("WebSocket Test!!!");
                auto func_ptr = func.lock();
                if (func_ptr && func_ptr->getPublisher(str)) {
                    return FHT::iTask::state::CONTINUE;
                }
                return FHT::iTask::state::FINISH;
            }, 5000);
        }
        catch (const std::exception& e) {
            FHT::LoggerStream::Log(FHT::LoggerStream::ERR) << METHOD_NAME << e.what();
        }
        return std::string();
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