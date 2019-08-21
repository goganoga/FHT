/***************************************
*      Framework Handler Task
*  https://github.com/goganoga/FHT
*  Created: 06.08.19
*  Copyright (C) goganoga 2019
***************************************/
#ifndef IHENDLER_H
#define IHENDLER_H
#include <functional>
#include <string>
#include <any>
struct iHendler {
	virtual ~iHendler() = default;
	struct data {
		int id = 0;
		std::string str0 = nullptr;
		std::string str1 = nullptr;
		std::string str2 = nullptr;
		double num = 0;
        std::any obj;

	};
	virtual void addUniqueHendler(std::string id, std::function<void(iHendler::data)> func) = 0;
	virtual bool removeUniqueHendler(std::string id) = 0;
	virtual std::function<void(iHendler::data)> getUniqueHendler(std::string id) = 0;

	virtual void addHendler(std::string id, std::function<void(void)> func) = 0;
	virtual bool removeHendler(std::string id) = 0;
    virtual std::function<void(void)> getHendler(std::string id) = 0;
};
#endif //IHENDLER_H
