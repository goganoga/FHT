/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 14.10.19
*  Copyright (C) goganoga 2019
***************************************/
#ifndef FHTWEBCLIENT_H
#define FHTWEBCLIENT_H
#include "FHT/Interface/Common/iClient.h"

#include <map>
#include <chrono>

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#ifdef WITH_COROUTINE
#include <boost/asio/spawn.hpp>
#else
#include <boost/asio/strand.hpp>
#endif

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
namespace ssl = boost::asio::ssl;       // from <boost/asio/ssl.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

namespace FHT {
    struct params {
        params(const iClient::httpClient& request, std::function<void(iClient::httpClient::httpResponse)> callback);
        std::string port;
        std::string host;
        std::string shema;
        std::string uri;
        const iClient::httpClient& m_request;
        std::function<void(iClient::httpClient::httpResponse)> m_callback;
    };
    struct iHttpConnect {
        bool ready = false;
    };
    struct webClient {
        webClient(iClient::httpClient& request, std::function<void(iClient::httpClient::httpResponse)> func, net::io_context& ioc);
        params m_params;
        ssl::context m_ctx{ ssl::context::tls_client };
        std::shared_ptr<iHttpConnect> m_http_connect;
    };
}
#endif //FHTWEBCLIENT_H
