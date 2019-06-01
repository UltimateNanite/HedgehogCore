#pragma once
#include "HCComponent.h"
typedef HCComponent* (* LoadComponentFn)(std::ifstream& ifs);

class Room;
class HCObject;
typedef HCComponent* (* CreateComponentFn)(Room* room, HCObject* parent);

class HCCompFactory
{
private:
	HCCompFactory();
	HCCompFactory(const HCCompFactory&) { }
	HCCompFactory& operator=(const HCCompFactory&) { return *this; }

	typedef std::map<std::string, std::pair<LoadComponentFn, CreateComponentFn>> FactoryMap;


	FactoryMap m_FactoryMap;
public:
	~HCCompFactory() { m_FactoryMap.clear(); }

	static HCCompFactory* Get()
	{
		static HCCompFactory instance;
		return &instance;
	}

	void Register(const std::string& compName, LoadComponentFn pfnLoad, CreateComponentFn pfnCreate);
	HCComponent* LoadComponent(std::ifstream& in, Room* room, HCObject* parent);
	HCComponent* CreateComponent(std::string in, Room* room, HCObject* parent);

	std::string GetComponentName(LoadComponentFn pfnLoad, CreateComponentFn pfnCreate);
};

