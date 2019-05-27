#pragma once
#include "Includes.h"
#include "Tile.h"
#include "HCObject.h"

#include "ParallaxLayer.h"
#include <unordered_map>
#include "Structs.h"

class Project;
struct TextureRefInfo;
struct ObjectAssociations
{
public:
	HCObject *parent;
};
typedef std::vector<std::vector<Tile>> TileData;
typedef std::unordered_map<std::string, sf::Texture> TextureData;
typedef std::unordered_map<std::string, sf::SoundBuffer> SoundData;

template <class N>
using Vector3D = std::vector<std::vector<std::vector<N>>>;

template <class N>
using StableVector3D = boost::container::stable_vector<boost::container::stable_vector<boost::container::stable_vector<N>>>;

template <class N>
using StableVector2D = boost::container::stable_vector<boost::container::stable_vector<N>>;
using Chunk = std::vector<std::vector<WorldTile>>;

class Room
{
public:
	std::string name;
	std::string path;
	std::string filename;
	Project *currentProject;
	std::vector<std::string> tilemapnames;
	std::unordered_map<std::string, TileData> tilemapdata; //Real tiles which chunks reference, order is: 1) tilemaps index, 2) tile x, 3) tile y
	Vector3D<Chunk> chunks;
	StableVector2D<HCObject> objects = { boost::container::stable_vector<HCObject>{} };
	std::map<int, ObjectAssociations> associations;
	std::vector<ParallaxLayer> parallaxLayers;
	std::string songfilename;
	//TextureData tilemaps;

	TextureData textures;
	SoundData sounds;

	sf::Vector2f startPosition = { 100, 100 };
	void AddTileMap(std::string tilemapname, bool forceRecreate = false);
	Room(Project* parent);
	Room();
	~Room();
};

std::ofstream& operator<<(std::ofstream& ofs, const Room& rt);
std::ifstream& operator>>(std::ifstream& ifs, Room& rt);


std::ofstream& operator<<(std::ofstream& ofs, const TileData& td);
std::ifstream& operator>>(std::ifstream& ifs, TileData& td);
