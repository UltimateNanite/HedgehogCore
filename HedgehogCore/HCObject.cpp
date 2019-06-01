#include "stdafx.h"
#include "HCObject.h"
#include "Room.h"
#include "HCComponent.h"

HCObject* NewObject(std::vector<HCComponent*> components, int layer, Room* currentRoom, std::string name) {
	if (layer < 0 || layer >= currentRoom->objects.size()) {
		std::cout << "Cannot place object on invalid layer.";
		return nullptr;
	}
	HCObject temp(name, { components });
	currentRoom->objects[layer].push_back(temp);

	HCObject *objInMem = &currentRoom->objects[layer][currentRoom->objects[layer].size() - 1];

	int ID = std::abs(std::rand());
	while (currentRoom->associations.count(ID) > 0)
		ID = std::abs(std::rand());

	void *IDPT = &ID;

	objInMem->AddComponent(new Renderer3D(objInMem, currentRoom));
	ObjectAssociations assoc;
	assoc.parent = objInMem;
	currentRoom->associations.insert(std::pair<int, ObjectAssociations>(ID, assoc));
	
	return objInMem;
};

boost::ptr_vector<HCComponent>* HCObject::GetComponentsPtr()
{
	return &components;
}

const boost::ptr_vector<HCComponent>* HCObject::GetConstComponentsPtr() const
{
	return &components;
}
HCObject::HCObject(std::string name)
{
	this->name = name;
}

HCObject::HCObject(std::string name, std::vector<HCComponent *> components) {
	for (auto& s : components) {
		this->components.push_back(s);
	}
	this->name = name;
}
int HCObject::Update(sf::Time deltaTime, Player &player)
{
	if (components.size() > 0) {
		for (int i = 0; i < components.size(); i++) {
			HCComponent* currentComponent = &components[i];
			currentComponent->Update(deltaTime, player); //TODO: add deltatime
		}
	}
	return 0;
}


HCObject::~HCObject()
{
}
void HCObject::AddComponent(HCComponent *component) {
	this->components.push_back(component);
}

sf::Vector2f HCObject::GetSize() const {
	return size;
}
void HCObject::SetSize(sf::Vector2f size)
{
	this->size = size;
}
sf::Vector2f HCObject::GetPosition() const {
	return position;
}

sf::FloatRect HCObject::getOutline() {
	return sf::FloatRect(this->position, this->GetSize());
}