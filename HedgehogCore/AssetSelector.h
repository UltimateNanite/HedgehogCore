#pragma once
/*
#include "Includes.h"
#include "Project.h"
struct AssetSelectorReturn {
	std::string path;
	int assetNo;
	int state;
	AssetSelectorReturn() : path(""), state(0), assetNo(0) {}
};
enum AssetType {
	Image = 1 << 0,
	Script = 1 << 1,
	Audio = 1 << 2,
	Tilemap = 1 << 3,
};
class AssetSelector {
private:
	int active = false;
	bool selection = false;
	std::vector<std::string> assetdisplays;
	std::vector<std::string> assetnames;

public:
	AssetSelectorReturn asset;
	AssetSelector(Project &currentProject, Room *currentRoom, int allowedtypes = AssetType::Script | AssetType::Audio | AssetType::Image | AssetType::Tilemap);
	AssetSelector();
	void Update(int index);
};*/