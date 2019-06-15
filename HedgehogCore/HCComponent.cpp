#include "stdafx.h"
#include "HCComponent.h"
#include "HCObject.h"
#include "Structs.h"
#include <imgui.h>


#ifdef HC_EDITOR
#include <imgui_stl.h>
#endif

#include "Room.h"
HCComponent::~HCComponent()
{
}



#ifdef HC_EDITOR
#ifndef IMGUI_EXTRA
#define IMGUI_EXTRA
namespace ImGui
{
	static auto vector_getter = [](void* vec, int idx, const char** out_text)
	{
		auto& vector = *static_cast<std::vector<std::string>*>(vec);
		if (idx < 0 || idx >= static_cast<int>(vector.size())) { return false; }
		*out_text = vector.at(idx).c_str();
		return true;
	};
	bool Combo(const char* label, int* currIndex, std::vector<std::string>& values)
	{
		if (values.empty()) { return false; }
		return Combo(label, currIndex, vector_getter,
			static_cast<void*>(&values), values.size());
	}

	bool ListBox(const char* label, int* currIndex, std::vector<std::string>& values)
	{
		if (values.empty()) { return false; }
		return ListBox(label, currIndex, vector_getter,
			static_cast<void*>(&values), values.size());
	}

}
#endif
#endif

std::ofstream& operator<<(std::ofstream& ofs, const HCComponent& comp) {
	return comp.filestream_out(ofs);
}
std::ifstream& operator>>(std::ifstream& ifs, HCComponent& comp) {
	return comp.filestream_in(ifs);
}
void HCComponent::draw(sf::RenderTarget& target, sf::RenderStates states) const 
{
}


void Renderer3D::Update(sf::Time dt, Player& player) {
	this->dt = dt;
}

void Renderer3D::im_draw()
{
}

Renderer3D::Renderer3D(HCObject *parent, Room *room) : HCComponent(parent, room)  {

	
	
}
void Renderer3D::Update(sf::Time dt)
{

}
void Renderer3D::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	// Enable position and color vertex components
	target.popGLStates();
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glVertexPointer(3, GL_FLOAT, 5 * sizeof(GLfloat), &points[0]);
	glTexCoordPointer(2, GL_FLOAT, 5 * sizeof(GLfloat), &points[0] + 3);
	glClearColor(0, 0, 0, 1);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);

	glDisable(GL_TEXTURE_2D);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Not needed since SFML handles clearing

	// Apply some transformations to rotate the cube
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(offset.x, offset.y, offset.z); //offset = {0,0,-200}
	glRotatef(dt.asSeconds() * 50, 1.f, 0.f, 0.f);
	glRotatef(dt.asSeconds() * 30, 0.f, 1.f, 0.f);
	glRotatef(dt.asSeconds() * 90, 0.f, 0.f, 1.f);
	// Draw the cube
	glDrawArrays(GL_TRIANGLES, 0, 36);
	
	//glLoadIdentity();
	target.pushGLStates();
}

HCComponent* new_clone(HCComponent const& other) {
	return other.clone();
}

void SpriteRenderer::Update(sf::Time dt)
{
	if (!txt_valid) {
		
		//Check if the texture is loaded
		txt_valid = room->textures.count(texturename) > 0;


		if (txt_valid) {
			txt = room->textures[texturename]; //Get the texture from the room's loaded assets
			if (parent->GetSize() == sf::Vector2f(0.f, 0.f))
				parent->SetSize(sf::Vector2f(txt.getSize()));
		}
	}

	if (wrapMode == WrapMode::Crop) {
		textureRect.width = parent->GetSize().x;
		textureRect.height = parent->GetSize().y;
	}
}

void SpriteRenderer::Update(sf::Time dt, Player& player) {
	Update(dt);
}

void SpriteRenderer::im_draw()
{
#ifdef HC_EDITOR
	ImGui::Text(("Texture: " + texturename).c_str());
	if (ImGui::Button("Change")) {
		ImGui::OpenPopup("Select asset##select comp texture");
	}
	if (ImGui::BeginPopup("Select asset##select comp texture"))
	{
		int index;
		std::vector<const char *> components = { };
		for (auto& s : room->textures) {
			components.push_back(s.first.c_str());
		}
		if (ImGui::ListBox("Type", &index, components.data(), components.size())) {
			texturename = components[index];
			ImGui::CloseCurrentPopup();
			txt_valid = false; //Reload texture
		}
		ImGui::EndPopup();
	}

	if (texturename != "") {
		
		std::vector<std::string> wrapModeStrings = { "Stretch", "Crop", "CropRepeat" };
		ImGui::Combo("Wrap mode", &wrapMode, wrapModeStrings);
		if (wrapMode != WrapMode::Stretch) {
			int rectArr[2] = { textureRect.left, textureRect.top };
			ImGui::InputInt2("Texture offset", rectArr);
			textureRect = sf::IntRect(rectArr[0], rectArr[1], parent->GetSize().x, parent->GetSize().y);
		}
	}
#endif
}

void SpriteRenderer::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	using namespace sf;


	Sprite spr;
	spr.setTexture(txt);

	spr.setPosition(parent->GetPosition());
	if (wrapMode == WrapMode::Stretch) {
		Vector2f scale;
		scale.x = parent->GetSize().x / txt.getSize().x;
		scale.y = parent->GetSize().y / txt.getSize().y;
		spr.setScale(scale);
	}
	else if (wrapMode == WrapMode::Crop) {
		
		spr.setTextureRect(textureRect);
		spr.setScale(1.f, 1.f);
	}
	target.draw(spr);
}

void ShapeRenderer::im_draw()
{
}

void ShapeRenderer::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
}

void SheetAnimator::Update(sf::Time dt)
{
	if (!renderer) {
		renderer = parent->GetComponent<SpriteRenderer>();
	}
	if (!renderer) return; //If there's still no SpriteRenderer, we shouldn't update anything

	
}

void SheetAnimator::Update(sf::Time dt, Player& player)
{
}

void SheetAnimator::im_draw()
{
#ifdef HC_EDITOR
	if (!renderer) {
		ImGui::Text("(!) SpriteRenderer not found.");
		return;
	}
	if (renderer->wrapMode != SpriteRenderer::WrapMode::Crop) {
		ImGui::Text("(!) SpriteRenderer should have wrap mode Crop for sprite sheet animation.");
		return;
	}

#endif
}

void SheetAnimator::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
}










//Saving/loading operator overloads
//TODO: Implement these



std::ofstream& Renderer3D::filestream_out(std::ofstream& ofs) const
{
	return ofs;
}
std::ifstream& Renderer3D::filestream_in(std::ifstream& ifs)
{
	return ifs;
}

HCComponent* Renderer3D::create(Room* room, HCObject* parent)
{
	return new Renderer3D(parent, room);
}

HCComponent* Renderer3D::factory_create(std::ifstream& ifs)
{
	Renderer3D *result = new Renderer3D();
	ifs >> *result;
	return result;
}



std::ifstream& SpriteRenderer::filestream_in(std::ifstream& ifs)
{
	return ifs;
}

HCComponent* SpriteRenderer::create(Room* room, HCObject* parent)
{
	return new SpriteRenderer(parent, room);
}

std::ofstream& SpriteRenderer::filestream_out(std::ofstream& ofs) const
{
	ofs << texturename;
	ofs << " | ";
	switch (wrapMode) {
	case WrapMode::Stretch:
		ofs << 1;
		break;
	case WrapMode::Crop:
		ofs << 2;
		ofs << ',' << textureRect.left << ',' << textureRect.top;
		break;
	case WrapMode::CropRepeat:
		ofs << 3;
		ofs << ',' << textureRect.left << ',' << textureRect.top;
		break;
	}
	return ofs;
}

HCComponent* SpriteRenderer::factory_create(std::ifstream& ifs)
{
	SpriteRenderer*result = new SpriteRenderer(nullptr, nullptr);

	std::string txtname;
	char in = '\0';
	for (int i = 0; i < 1000; i++) {
		ifs >> in; 
		if (in == '|') 
			break;
		txtname += in;
	}
	result->texturename = txtname;
	int mode = 0;
	ifs >> mode;
	if (mode == 0)
		throw std::exception("Failed loading SpriteRenderer");

	switch (mode) {
	case 1:
		result->wrapMode = WrapMode::Stretch;
		break;
	case 2:
		result->wrapMode = WrapMode::Crop; 
		ifs >> in >> result->textureRect.left >> in >> result->textureRect.top;
		break;
	case 3:
		result->wrapMode = WrapMode::CropRepeat;
		ifs >> in >> result->textureRect.left >> in >> result->textureRect.top;
		break;
	}
	return result;
}


std::ofstream& SheetAnimator::filestream_out(std::ofstream& ofs) const
{
	return ofs;
}
std::ifstream& SheetAnimator::filestream_in(std::ifstream& ifs)
{
	return ifs;
}

HCComponent* SheetAnimator::create(Room* room, HCObject* parent)
{
	return new SheetAnimator(parent, room);
}

HCComponent* SheetAnimator::factory_create(std::ifstream& ifs)
{
	return nullptr;
}





bool ObjectCollidable::CheckCollision(sf::FloatRect collider, PlayerState state)
{
	return false;
}

short ObjectCollidable::GetHeight(sf::Vector2f position, int groundMode)
{
	return 0;
}

void CollisionHazard::Update(sf::Time dt)
{
	return;
}

void CollisionHazard::Update(sf::Time dt, Player& player)
{
	sf::FloatRect thisCol(parent->GetPosition(), parent->GetSize());

	if (player.colliders.leftCeilCol.intersects(thisCol) ||
		player.colliders.rightCeilCol.intersects(thisCol) ||
		player.colliders.leftFootCol.intersects(thisCol) ||
		player.colliders.rightFootCol.intersects(thisCol) ||
		player.colliders.pushCol.intersects(thisCol)) {

		player.hurt(room, parent);
	}
}

void CollisionHazard::im_draw()
{
#ifdef HC_EDITOR
	ImGui::Text("(No settings for CollisionHazard)");
#endif
}

std::ofstream& CollisionHazard::filestream_out(std::ofstream& ofs) const
{
	return ofs;
}

std::ifstream& CollisionHazard::filestream_in(std::ifstream& ifs)
{
	return ifs;
}

HCComponent* CollisionHazard::create(Room* room, HCObject* parent)
{
	return new CollisionHazard(parent, room);
}

HCComponent* CollisionHazard::factory_create(std::ifstream& ifs)
{
	return new CollisionHazard(nullptr, nullptr);
}

void PythonScript::LoadScript()
{
	
	pName = PyUnicode_FromString(filename.c_str());
	pModule = PyImport_Import(pName);
	if (pModule != NULL)
		valid = true;
	else {
		std::cout << "Error: Failed to load Python script " << filename << "\n";
		PyErr_Print();
	}
	Py_DECREF(pName);
}

void PythonScript::CallFunction(std::string name, std::vector<std::pair<PyType, std::string>> args)
{
	int errorcheck = PyModule_AddObject(pModule, "parent", PyUnicode_FromString(parent->name.c_str()));
	if (errorcheck == -1)
	std::cout << "Failed to set parent name in Python.\n";
	PyObject * pFunc, * pArgs, * pValue = NULL;
	pFunc = PyObject_GetAttrString(pModule, name.c_str());

	if (pFunc && PyCallable_Check(pFunc)) {
		pArgs = PyTuple_New(args.size());

		for (int i = 0; i < args.size(); i++) {
			
			switch (args[i].first) {

			case PyType::Long:
				pValue = PyLong_FromString(args[i].second.c_str(), NULL, 10);
				break;

			case PyType::Bool:
				if (args[i].second == "true")
					pValue = PyBool_FromLong(1);
				else if (args[i].second == "false")
					pValue = PyBool_FromLong(0);
				else
					pValue = NULL;
				break;

			case PyType::Float:
				pValue = PyFloat_FromString(PyUnicode_FromString(args[i].second.c_str()));
				break;

			case PyType::Null:
				pValue = NULL;
				break;

			default:
				break;

			}
			if (!pValue) {
				Py_DECREF(pArgs);
				Py_DECREF(pModule);
				fprintf(stderr, "Cannot convert argument\n");
				return;
			}
			PyTuple_SetItem(pArgs, i, pValue);
		}

		pValue = PyObject_CallObject(pFunc, pArgs);

		if (PyErr_Occurred())
			PyErr_Print();

		Py_DECREF(pArgs);

		Py_DECREF(pFunc);
	}
}

void PythonScript::Update(sf::Time dt)
{
	
	if (!valid) {
		LoadScript();
		if (!valid) 
			return;
		else 
			CallFunction("setup");
	}
	if (running)
		CallFunction("update", { std::make_pair(PyType::Float, std::to_string(dt.asSeconds())) });
}

void PythonScript::Update(sf::Time dt, Player& player)
{
	Update(dt);
}

void PythonScript::im_draw()
{
#ifdef HC_EDITOR
	ImGui::Text(("Script: " + filename).c_str());
	
	if (valid) {
		ImGui::Text("Valid module");
	}
	else {
		ImGui::Text("(!) Invalid module");
	}

	if (ImGui::Button("Play")) {
		running = true;
	}
	ImGui::SameLine();
	if (ImGui::Button("Pause")) {
		running = false;
	}
	ImGui::SameLine();
	if (ImGui::Button("Stop")) {
		running = false;
		CallFunction("setup");
	}
#endif
}

void PythonScript::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
}

std::ofstream& PythonScript::filestream_out(std::ofstream& ofs) const
{
	ofs << filename;
	return ofs;
}

std::ifstream& PythonScript::filestream_in(std::ifstream& ifs)
{
	char in = '\0';
	while (in != ',') {
		ifs >> in;
		if (in == ',') break;
		filename += in;
	}
	this->filename = filename;
	return ifs;
}

HCComponent* PythonScript::create(Room* room, HCObject* parent)
{
	return new PythonScript(parent, room);
}

HCComponent* PythonScript::factory_create(std::ifstream& ifs)
{
	PythonScript* comp = new PythonScript(nullptr, nullptr);
	std::string filename;
	char in = '\0';

	while (in != ',') {
		ifs >> in;
		if (in == ',') break;
		filename += in;
	}
	ifs.unget();
	
	comp->filename = filename;
	
#ifdef HC_CORE
	comp->running = true;
#endif
	return comp;
}

