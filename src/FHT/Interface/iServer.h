/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 06.08.19
*  Copyright (C) goganoga 2019
***************************************/
#ifndef FHTISERVER_H
#define FHTISERVER_H
#include <memory>
#include <cstdint>
namespace FHT {
    struct iServer {
		enum Type {SYNC, ASYNC};
        virtual void run() = 0;
        virtual void setPort(std::uint16_t port_) = 0;
        virtual void setHost(std::string host_) = 0;
        //return Hendler lessen name
        virtual std::string lessenAll(bool flag) = 0;
		virtual void setTypeProcessorHandler(Type type) = 0;
        virtual ~iServer() = default;
    };

    static std::string webSocket(std::string str) { return "ws" + str; };

    struct wsSubscriber {
        friend class Server;
        wsSubscriber() {}
        ~wsSubscriber() {
            publisher = nullptr;
            subscriber = nullptr;
            deleter = nullptr;
        }
        void setSubscriber(std::function<void(std::string&)>& subscriberFunctor) {
            subscriber = std::move(subscriberFunctor);
        };
        void setSubscriber(std::function<void(std::string&)> subscriberFunctor) {
            subscriber = std::move(subscriberFunctor);
        };
        void setSubscriber(void(*subscriberFunctor)(std::string&)) {
            subscriber = static_cast<std::function<void(std::string&)>>(subscriberFunctor);
        };
        bool getPublisher(std::string& str) {
            return publisher ? publisher(str) : false;
        }
    private:
        std::function<void()> deleter;
        std::function<bool(std::string&)> publisher;
        std::function<void(std::string&)> subscriber;
    };
}
#endif //FHTISERVER_H
