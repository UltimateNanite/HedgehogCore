#pragma once
#include "Includes.h"

#include "HCComponent.h"
//#include "AssetSelector.h"
class Room;
struct Player;


HCObject *NewObject(std::vector<HCComponent *> components, int layer, Room *currentRoom, std::string name = "New Object");
class HCObject
{
private:
	sf::Vector2f size;
	HCComponent* currentComponent;
	boost::ptr_vector<HCComponent> components;
public:
	boost::ptr_vector<HCComponent> *GetComponentsPtr() {
		return &components;
	}

	float activradius = 500;
	bool colliding;
	bool layerIndependent;
	HCObject(std::string name);
	HCObject(std::string name, std::vector<HCComponent *> components);
	int Update(sf::Time deltaTime, Player& player);
	
	sf::Vector2f startposition;
	sf::Vector2f position;
	
	~HCObject();

	void AddComponent(HCComponent *component);
	std::string name;
	sf::Vector2f GetSize();
	void SetSize(sf::Vector2f size);
	sf::Vector2f GetPosition();
	virtual sf::FloatRect getOutline();

	template<typename T>
	T* GetComponent() {
		for (auto& component : components) {
			if (dynamic_cast<T*>(&component)) {
				return dynamic_cast<T*>(&component);
			}
		}
		return nullptr;
	}

	template<typename T>
	std::vector<T> GetComponents() {
		std::vector<T> result;
		for (auto& component : components) {
			if (dynamic_cast<T*>(&component)) {
				result.push_back(dynamic_cast<T>(component));
			}
		}
		return result;
	}

	template<typename T>
	bool HasComponent() {
		for (auto& component : components) {
			if (dynamic_cast<T*>(&component)) {
				return true;
			}
		}
		return false;
	}
};

