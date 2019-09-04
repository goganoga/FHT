/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 06.08.19
*  Copyright (C) goganoga 2019
***************************************/
#include "Test.h"
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
		H->addUniqueHendler("/test", std::bind(&Test::mainTest, this, std::placeholders::_1));
	}
	std::string Test::mainTest(FHT::iHendler::data resp) {
		std::string buf = resp.str0;
		std::string location = resp.str1;
		FHT::iHendler::FHT_MAP headers = *(FHT::iHendler::FHT_MAP*)resp.obj2;
		std::string postBody = resp.str2;

		std::string body;
		try {
			FHT::iHendler::FHT_MAP postParam(postBody);
			std::string param_test("null");
			auto test = headers.find("qq");
			if (test != nullptr) param_test = test;
			std::string h_sha256 = ",\"hash_sha256\":\"" + hash(buf.c_str()) + "\"";
			std::string h_md5 = ",\"hash_md5\":\"" + md5_hash(buf.c_str()) + "\"";
			std::string h_guid = ",\"guid\":\"" + guid() + "\"";
			std::string h_guid_nil = ",\"guid_location\":\"" + guid(buf) + "\"";
			std::string h_sha512 = ",\"hash_sha512\":\"" + hash512(buf.c_str()) + "\"";
			std::string h_sha512_gen = ",\"h_sha512_gen\":\"" + gen() + "\"";
			std::string h_ = buf;
			h_.append(md5_hash(buf.c_str()));
			h_.append(std::to_string(time(nullptr)));
			std::string h = hash(h_.c_str());
			std::string h_h = ",\"hash_h\":\"" + h + "\"";

			body = "{\"status\":1,\"uri\":\"" + buf + "\"" + ", \"location\":\"" + location + "\"" + ", \"postBody\":\"" + postBody + "\"" +
				",\"param[qq]\":\"" + param_test + "\"" +
				(h_sha256 + h_md5 + h_guid) +
				(h_guid_nil + h_sha512) +
				(h_sha512_gen + h_h) + "}";
		}
		catch (const std::exception& e) {
			std::cerr << "Error: " << e.what() << std::endl;
		}
		return body;
	}

	std::string Test::md5_hash(const char* string) {
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
}