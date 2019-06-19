#include "stdafx.h"
#include "HCComponent.h"
#include "HCObject.h"
#include "Structs.h"
#include "Room.h"
#include "imgui.h"

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



void SpriteRenderer::im_draw()
{
	ImGui::Text(("Texture: " + texturename).c_str());
	if (ImGui::Button("Change")) {
		ImGui::OpenPopup("Select asset##select comp texture");
	}
	if (ImGui::BeginPopup("Select asset##select comp texture"))
	{
		int index;
		std::vector<const char*> components = { };
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
}


void SheetAnimator::im_draw()
{
	if (!renderer) {
		ImGui::Text("(!) SpriteRenderer not found.");
		return;
	}
	if (renderer->wrapMode != SpriteRenderer::WrapMode::Crop) {
		ImGui::Text("(!) SpriteRenderer should have wrap mode Crop for sprite sheet animation.");
		return;
	}

}

void CollisionHazard::im_draw()
{

	ImGui::Text("(No settings for CollisionHazard)");

}


void PythonScript::im_draw()
{
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
}