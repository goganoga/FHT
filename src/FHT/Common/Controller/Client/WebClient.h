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
        webClient(std::string url, std::string body, std::function<void(FHT::iClient::respClient)>* func, event_base* base);
        ~webClient();
    private:
        static void httpRequestDone(struct evhttp_request* req, void* ctx);
        static std::function<void(FHT::iClient::respClient)> funcCallback;
        enum hostnameValidation {
            MatchFound,
            MatchNotFound,
            NoSANPresent,
            MalformedCertificate,
            Error
        };
        static hostnameValidation matchesSubjectAlternativeName(const char* hostname, const X509* serverCert);
        static hostnameValidation matchesCommonName(const char* hostname, const X509* serverCert);
        static hostnameValidation validateHostname(const char* hostname, const X509* serverCert);
#ifdef _WIN32
        int addCertForStore(X509_STORE* store, const char* name);
#endif
        static int certVerifyCallback(X509_STORE_CTX* x509Ctx, void* arg);
    };
}
#endif //FHTWEBCLIENT_H
