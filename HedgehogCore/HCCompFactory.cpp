#include "HCCompFactory.h"
#include "Room.h"
#include "HCObject.h"
void HCCompFactory::Register(const std::string& compName, LoadComponentFn pfnLoad, CreateComponentFn pfnCreate) {
	m_FactoryMap[compName] = std::pair<LoadComponentFn, CreateComponentFn>(pfnLoad, pfnCreate);
}


HCComponent* HCCompFactory::LoadComponent(std::string name, std::ifstream& in, Room* room, HCObject* parent)
{
	FactoryMap::iterator it = m_FactoryMap.find(name);
	if (it != m_FactoryMap.end()) {
		HCComponent* result = it->second.first(in);
		result->parent = parent;
		result->room = room;
		return result;
	}
	return NULL;
}

HCComponent* HCCompFactory::CreateComponent(std::string in, Room* room, HCObject* parent)
{

	FactoryMap::iterator it = m_FactoryMap.find(in);
	if (it != m_FactoryMap.end()) {
		HCComponent* result = it->second.second(room, parent);

		return result;
	}
	return NULL;
}

HCCompFactory::HCCompFactory()
{
	//Register("Renderer3D", &Renderer3D::factory_create, &Renderer3D::create);
	Register("CollisionHazard", &CollisionHazard::factory_create, &CollisionHazard::create);
	Register("SpriteRenderer", &SpriteRenderer::factory_create, &SpriteRenderer::create);
	Register("SheetAnimator", &SheetAnimator::factory_create, &SheetAnimator::create);
	Register("PythonScript", &PythonScript::factory_create, &PythonScript::create);
	//Register("ShapeRenderer", &ShapeRenderer::factory_create, &ShapeRenderer::create);
}


std::string HCCompFactory::GetComponentName(LoadComponentFn pfnLoad, CreateComponentFn pfnCreate) {
	for (auto& fn : m_FactoryMap) {
		if (fn.second.first == pfnLoad && fn.second.second == pfnCreate)
			return fn.first;
	}
	return "";
}