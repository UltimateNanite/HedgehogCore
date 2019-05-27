#include "stdafx.h"
#include "AssetSelector.h"
/*
AssetSelector::AssetSelector() {

}
AssetSelector::AssetSelector(Project &currentProject, Room *currentRoom, int allowedtypes) {
	this->asset = AssetSelectorReturn();
	if ((AssetType::Script & allowedtypes) == AssetType::Script)
		for (auto s : currentProject.scriptnames) {
			assetdisplays.push_back(s + ".cs");
			assetnames.push_back(s);
		}
	if ((AssetType::Image & allowedtypes) == AssetType::Image)
		for (auto s : currentRoom->textures) {
			assetdisplays.push_back(s.first);
			assetnames.push_back(s.first);
		}
	if ((AssetType::Audio & allowedtypes) == AssetType::Audio)
		for (auto s : currentRoom->sounds) {
			assetdisplays.push_back(s.first);
			assetnames.push_back(s.first);
		}
	if ((AssetType::Tilemap & allowedtypes) == AssetType::Tilemap)
		for (auto s : currentRoom->tilemapnames) {
			assetdisplays.push_back(s);
			assetnames.push_back(s);
		}
	bool assetselected = false;
}
void AssetSelector::Update(int index) {
	std::string windowname = "";
	if (assetdisplays.size() > 0) {
		windowname = ("Asset Selector" + std::string(index, ' '));
	}
	else {
		windowname = ("Asset Selector (No imported assets that meet the criteria)" + std::string(index, ' '));
	}
	if (ImGui::Begin(windowname.c_str())) {



		static int listbox_item_current = 1;
		if (ImGui::ListBox("   ", &listbox_item_current, assetdisplays)) {
			selection = true;
		}
		if (selection) {
			if (ImGui::Button("Select")) {
				asset.path = assetnames[listbox_item_current];
				asset.assetNo = listbox_item_current;
				asset.state = 1;
			}
			ImGui::SameLine();
		}
		if (ImGui::Button("Cancel")) {
			asset.state = -1;
		}
		ImGui::End();
	}
}*/