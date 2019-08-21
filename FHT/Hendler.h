#ifndef HENDLER_H
#define HENDLER_H
#include "iHendler.h"
#include <map>
class Hendler: public iHendler
{
public:
	Hendler();
	void addUniqueHendler(std::string id, std::function<void(iHendler::data)> func) override final;
	void addHendler(std::string id, std::function<void(void)> func) override final;
	bool removeUniqueHendler(std::string id) override final;
	bool removeHendler(std::string id) override final;
	std::function<void(iHendler::data)> getUniqueHendler(std::string id) override final;
	std::function<void(void)> getHendler(std::string id) override final;
    virtual ~Hendler() override;
private:
	std::map<std::string, std::function<void(iHendler::data)>> mapHendler;
	std::map<std::string, std::function<void(void)>> mapList;
};
#endif //HENDLER_H
