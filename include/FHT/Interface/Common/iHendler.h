/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 06.08.19
*  Copyright (C) goganoga 2019
***************************************/
#ifndef FHTIHENDLER_H
#define FHTIHENDLER_H
#include <functional>
#include <string>
#include <cstring>
#include <memory>
#include <any>
#include <map>

#include <boost/beast/core/multi_buffer.hpp>
#include <boost/beast/core/buffers_to_string.hpp>
#include <boost/beast/http/fields.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/exception/to_string.hpp>

namespace FHT{
    struct iHendler {
        virtual ~iHendler() = default;
        struct dataRequest {
            std::map<std::string, std::string> params;
            std::map<std::string, std::string> headers;
            std::string uri;
            std::string nextLocation;
            std::string ipClient;
            int portClient = 0;

            boost::beast::multi_buffer body;
            inline std::string getStringFromBody() {
                return boost::beast::buffers_to_string(body.data());
            }

            std::any WSInstanse;
            void* ptr;
        };
        struct dataResponse {
            boost::beast::multi_buffer body;
            std::map<std::string, std::string> headers;
            inline void setStringToBody(std::string body_str) {
                headers.emplace(boost::beast::http::to_string(boost::beast::http::field::content_type), "text/plain");
                body.commit(boost::asio::buffer_copy(body.prepare(body_str.size()), boost::asio::buffer(body_str)));
            }
        };
        using uniqueHendler = std::function<dataResponse(iHendler::dataRequest&)>;
        virtual void addUniqueHendler(std::string id, uniqueHendler func) = 0;
        virtual bool removeUniqueHendler(std::string id) = 0;
        virtual std::shared_ptr<uniqueHendler> getUniqueHendler(std::string id) = 0;

        virtual void addHendler(std::string id, std::function<void(void)> func) = 0;
        virtual bool removeHendler(std::string id) = 0;
        virtual std::shared_ptr<std::function<void(void)>> getHendler(std::string id) = 0;
    };
}
#endif //FHTIHENDLER_H
