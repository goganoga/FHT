/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 14.10.19
*  Copyright (C) goganoga 2019
***************************************/
#include "FHT/Common/Controller/Client/Client.h"
#include "FHT/Common/Controller/Controller.h"
#include "FHT/Common/Controller/Client/WebClient.h"
#include "FHT/LoggerStream.h"

#include <future>

namespace FHT {
    std::shared_ptr<Client> Client::getClient() {
        auto static a = std::make_shared<Client>();
        return a;
    }
    Client::Client(){
    }
    Client::~Client(){
    }

    void iClient::httpClient::fetch(std::function<void(httpResponse)> callback) {
        Client::getClient()->fetch(*this, callback);
    }

    const iClient::httpClient::httpResponse iClient::httpClient::fetch() {
        return Client::getClient()->fetch(*this);
    }

    void Client::fetch(iClient::httpClient req, std::function<void(iClient::httpClient::httpResponse)> callback) {
        // move to class Client
        net::io_context m_ioc;
        // net::io_context& m_ioc = client_ioc;
        std::string &url = req.url;
        if (url.empty() || url.length() < 6 || (url.substr(0, 7) != "http://" && url.substr(0, 8) != "https://")) {
            FHT::LoggerStream::Log(FHT::LoggerStream::ERR) << METHOD_NAME << "No correct url";
            callback({ -1, "No correct url" });
            return;
        }
        auto a = std::make_unique<webClient>(req, callback, m_ioc);
        //Push to Task lambda with m_ioc.poll();
        m_ioc.run();
    }

    const iClient::httpClient::httpResponse Client::fetch(iClient::httpClient& req) {
        net::io_context m_ioc;
        std::string& url = req.url;
        if (url.empty() || url.length() < 6 || (url.substr(0, 7) != "http://" && url.substr(0, 8) != "https://")) {
            FHT::LoggerStream::Log(FHT::LoggerStream::ERR) << METHOD_NAME << "No correct url";
            return { -1, "No correct url" };
        }
        iClient::httpClient::httpResponse result;
        std::function<void(iClient::httpClient::httpResponse)> func([&](iClient::httpClient::httpResponse a) {
            result = a;
        });

        auto a = std::make_unique<webClient>(req, func, m_ioc);
        m_ioc.run();

        return result;
    }
}
