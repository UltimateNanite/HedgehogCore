#include "HCAsset.h"


template<typename AssetType>
HCAsset<AssetType>::HCAsset(AssetType asset) : asset(asset)
{
}

template<typename AssetType>
HCAsset<AssetType>::~HCAsset()
{
}
template<typename AssetType>
AssetType HCAsset<AssetType>::getAsset() const {
	return asset;
}

template<typename AssetType>
std::string HCAsset<AssetType>::getAssetType() const {
	return typeid(AssetType).name();
}


//Specializations
std::string HCAsset<sf::Texture>::getAssetType() const {
	return "Texture";
}

std::string HCAsset<sf::Sound>::getAssetType() const {
	return "Sound";
}