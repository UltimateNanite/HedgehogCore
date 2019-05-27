#include "stdafx.h"
#include "HCComponent.h"
#include "HCObject.h"
#include "Structs.h"
#include <imgui.h>

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
//TODO: Use these

std::ofstream& Renderer3D::operator<<(std::ofstream& ofs)
{
	return ofs;
}
std::ifstream& Renderer3D::operator>>(std::ifstream& ifs)
{
	return ifs;
}


std::ofstream& SpriteRenderer::operator<<(std::ofstream& ofs)
{
	return ofs;
}
std::ifstream& SpriteRenderer::operator>>(std::ifstream& ifs)
{
	return ifs;
}


std::ofstream& SheetAnimator::operator<<(std::ofstream& ofs)
{
	return ofs;
}
std::ifstream& SheetAnimator::operator>>(std::ifstream& ifs)
{
	return ifs;
}

bool ObjectCollidable::CheckCollision(sf::FloatRect collider, PlayerState state)
{
	return false;
}

short ObjectCollidable::GetHeight(sf::Vector2f position, int groundMode)
{
	return 0;
}
