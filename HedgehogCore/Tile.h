#pragma once
#ifndef TILE_H
#define TILE_H
#include <unordered_map>
#include "Includes.h"

struct TextureRefInfo;
struct TileHeightInfo {
	std::vector<unsigned char> down;
	std::vector<unsigned char> right;
	std::vector<unsigned char> up;
	std::vector<unsigned char> left;
};
class Room;
class Tile
{
private:
	sf::VertexArray quad;
	bool drawReady = false;
	std::unordered_map<std::string, sf::Texture> *txts;
	std::string texturename;
	TileHeightInfo heightArrays;
	sf::Vector2<unsigned short> textureRectIndex;
public:
	sf::VertexArray draw(sf::Vector2i position, sf::RenderTarget& target, sf::Color multCol = sf::Color::White);
	TextureRefInfo GetRefInfo();
	Tile();
	~Tile();
	float GetHeight(float index, int groundMode, bool smooth = true);
	std::vector<sf::Uint8>* GetHeightArray(int groundMode);
	void SetHeightArray(short *heightArray, int groundMode);
	sf::Texture* GetTexture();
	sf::IntRect GetRect();
	void SetRect(sf::Vector2i index);
	float GetAngle(int groundMode = 0);
	friend class Room;
	friend class LevelEditor;
	friend std::ofstream& operator<<(std::ofstream& ofs, const std::vector<std::vector<Tile>>& td);
	friend std::ifstream& operator>>(std::ifstream& ifs, std::vector<std::vector<Tile>>& td);
	friend std::ofstream& operator<<(std::ofstream& ofs, const Room& rt);
	static void SetTileDataFilename(std::vector<std::vector<Tile>>& td, std::string name);
	static void SetTxtsPointer(std::vector<std::vector<Tile>>& td, Room *currentRoom);
	static sf::RectangleShape floatRectToRect(sf::FloatRect rect);
};

std::vector<unsigned char> GenerateHeightArray(sf::Image arrayImg, sf::Vector2i pos, int groundMode);
#endif