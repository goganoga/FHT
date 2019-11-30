/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 14.10.19
*  Copyright (C) goganoga 2019
***************************************/
#ifndef FHTWEBCLIENT_H
#define FHTWEBCLIENT_H
#include "Controller/Server/InitialSer.h"
#include "iClient.h"
#include "iController.h"
#include <openssl/x509v3.h>
namespace FHT {
	struct webClient {
		webClient(std::string url, std::string body, std::function<void(FHT::iClient::respClient)>* func);
		~webClient();
	private:
        static void httpRequestDone(struct evhttp_request* req, void* ctx);
        static std::function<void(FHT::iClient::respClient)> func_;
        enum hostnameValidationResult {
			MatchFound,
			MatchNotFound,
			NoSANPresent,
			MalformedCertificate,
			Error
		};
        static hostnameValidationResult matchesSubjectAlternativeName(const char* hostname, const X509* serverCert);
        static hostnameValidationResult matchesCommonName(const char* hostname, const X509* serverCert);
        static hostnameValidationResult validate_hostname(const char* hostname, const X509* serverCert);
#ifdef _WIN32
        int addCertForStore(X509_STORE* store, const char* name);
#endif
        static int cert_verify_callback(X509_STORE_CTX* x509Ctx, void* arg);
	};
}
#endif //FHTWEBCLIENT_H
