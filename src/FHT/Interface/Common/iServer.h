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
        virtual void run() = 0;
        virtual void setWorker(int worker) = 0;
        virtual void setPort(int port) = 0;
        virtual void setHost(std::string host) = 0;
        //return Hendler lessen name
        virtual std::string lessenAll(bool flag) = 0;
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
            close = nullptr;
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
        void setDeleter(std::function<void()>& deleterFunctor) {
            deleter = std::move(deleterFunctor);
        };
        void setDeleter(std::function<void()> deleterFunctor) {
            deleter = std::move(deleterFunctor);
        };
        void setDeleter(void(*deleterFunctor)()) {
            deleter = static_cast<std::function<void()>>(deleterFunctor);
        };
        bool getPublisher(std::string& str) {
            return publisher ? publisher(str) : false;
        };
        void stop() {
            if(close) (*close.get())();
        };
        
    private:
        std::function<void()> deleter;
        std::function<bool(std::string&)> publisher;
        std::function<void(std::string&)> subscriber;
        std::shared_ptr<std::function<void()>> close;
    };
}
#endif //FHTISERVER_H
