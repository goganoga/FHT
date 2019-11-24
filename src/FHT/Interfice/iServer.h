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
		virtual void setPort(std::uint16_t port_) = 0;
		virtual void setHost(std::string host_) = 0;
		//return Hendler lessen name
		virtual std::string lessenAll(bool flag) = 0;
		virtual ~iServer() = default;
	};

	static std::string webSocket(std::string str) { return "ws" + str; };

	struct wsSubscriber {
		friend class Server;
		wsSubscriber(
			std::function<bool(std::string&)> publisher_ = nullptr,
			std::function<void()> deleter_ = nullptr,
			std::function<void(std::string&)> subscriber_ = nullptr) :
			publisher(publisher_),
			deleter(deleter_),
			subscriber(subscriber_) {}
		void setSubscriber(std::function<void(std::string&)>& subscriberFunctor) {
			if (!isDisconnect) subscriber = std::move(subscriberFunctor);
		};
		~wsSubscriber() {
			//delete this;
		}
		void setSubscriber(std::function<void(std::string&)> subscriberFunctor) {
			if (!isDisconnect) subscriber = std::move(subscriberFunctor);
		};
		void setSubscriber(void(*subscriberFunctor)(std::string&)) {
			if(!isDisconnect) subscriber = static_cast<std::function<void(std::string&)>>(subscriberFunctor);
		};
		bool getPublisher(std::string& str) {
			return isDisconnect ? false : publisher ? publisher(str) : false;
		}
	private:
		bool isDisconnect = false;
		std::function<void()> deleter;
		std::function<bool(std::string&)> publisher;
		std::function<void(std::string&)> subscriber;
	};
}
#endif //FHTISERVER_H
