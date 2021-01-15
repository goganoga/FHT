/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 14.10.19
*  Copyright (C) goganoga 2019
***************************************/
#ifndef FHTICLIENT_H
#define FHTICLIENT_H
#include <memory>
#include <cstdint>
#include <functional>
#include <map>

#include <boost/beast/core/multi_buffer.hpp>
#include <boost/beast/core/buffers_to_string.hpp>
#include <boost/beast/http/fields.hpp>
#include <boost/beast/http/verb.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/exception/to_string.hpp>

namespace FHT {
    struct iClient {
        struct httpClient {
            struct httpResponse {
                int status = 0;
                std::string err;
                boost::beast::multi_buffer body;
                std::map<std::string, std::string> headers;
                inline std::string getStringFromBody() {
                    return boost::beast::buffers_to_string(body.data());
                }
            };
            std::string url;
            boost::beast::multi_buffer body;
            std::map<std::string, std::string> headers;
            boost::beast::http::verb type = boost::beast::http::verb::get;
            inline void setStringToBody(std::string body_str) {
                headers.emplace(boost::beast::http::to_string(boost::beast::http::field::content_type), "text/plain");
                body.commit(boost::asio::buffer_copy(body.prepare(body_str.size()), boost::asio::buffer(body_str)));
                type = boost::beast::http::verb::post;
            }

            //async fetch
            void fetch(std::function<void(httpResponse)>);

            //sync fetch
            const httpResponse fetch();
        };
        virtual ~iClient() = default;
    };
}
#endif //FHTICLIENT_H
