#pragma once
#include "Includes.h"
template<typename AssetType>
class HCAsset
{
private:
	AssetType asset;
public:
	HCAsset(AssetType asset);
	~HCAsset();
	AssetType getAsset() const;
	std::string getAssetType() const;
};

typedef HCAsset<sf::Texture> HCTextureAsset;
typedef HCAsset<sf::Sound> HCSoundAsset;