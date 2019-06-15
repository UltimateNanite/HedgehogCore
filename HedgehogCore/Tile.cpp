#include "stdafx.h"
#include "Tile.h"
#include <array>
#include "Structs.h"
#include "Room.h"
Tile::Tile()
{
}


Tile::~Tile()
{
}
sf::VertexArray Tile::draw(sf::Vector2i position, sf::RenderTarget& target, sf::Color multCol) {
	/*sf::Sprite spr;
	spr.setTexture(this->txts->at(texturename), true);
	spr.setTextureRect(this->GetRect());
	spr.setPosition(sf::Vector2f(position));
	spr.setColor(multCol);
	target.draw(spr);*/
	
	if (!drawReady) {
		quad = sf::VertexArray(sf::Quads, 4);
		quad[0].texCoords = sf::Vector2f(textureRectIndex.x * 16, textureRectIndex.y * 16);
		quad[1].texCoords = sf::Vector2f(textureRectIndex.x * 16 + 16, textureRectIndex.y * 16);
		quad[2].texCoords = sf::Vector2f(textureRectIndex.x * 16 + 16, textureRectIndex.y * 16 + 16);
		quad[3].texCoords = sf::Vector2f(textureRectIndex.x * 16, textureRectIndex.y * 16 + 16);
	}
	quad[0].position = sf::Vector2f(position);
	quad[1].position = sf::Vector2f(position + sf::Vector2i(16, 0));
	quad[2].position = sf::Vector2f(position + sf::Vector2i(16, 16));
	quad[3].position = sf::Vector2f(position + sf::Vector2i(0, 16));
	return quad;
}
std::vector<unsigned char> GenerateHeightArray(sf::Image arrayImg, sf::Vector2i pos, int groundMode) {
	

	std::array<short, 16> temp{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	if (groundMode == 0 || groundMode == 2) {
		for (int x = 0; x < 16; x++) {
			bool pixelFound = false;

			if (groundMode == 0) {
				for (int y = 0; y < 16 && !pixelFound; y++) {
					if (arrayImg.getPixel(pos.x * 16 + x, pos.y * 16 + y).a >= 0.5f) {
						temp[x] = 16 - y;
						pixelFound = true;
					}
				}
			}
			else {
				for (int y = 15; y >= 0 && !pixelFound; y--) {
					sf::Color temp2 = arrayImg.getPixel(pos.x * 16 + x, pos.y * 16 + y);
					if (temp2.a >= 0.5f) {
						temp[x] = y + 1;
						pixelFound = true;
					}
				}
			}

		}

	}
	else if (groundMode == 3 || groundMode == 1) {
		if (groundMode == 3) {
			for (int y = 0; y < 16; y++) {
				bool pixelFound = false;
				for (int x = 0; x < 16 && !pixelFound; x++) {
					if (arrayImg.getPixel(pos.x * 16 + x, pos.y * 16 + y).a >= 0.5f) {
						temp[y] = 16 - x;
						pixelFound = true;
					}
				}
			}
		}
		else {
			for (int y = 0; y < 16; y++) {
				bool pixelFound = false;
				for (int x = 15; x >= 0 && !pixelFound; x--) {
					if (arrayImg.getPixel(pos.x * 16 + x, pos.y * 16 + y).a >= 0.5f) {
						temp[y] = x + 1;
						pixelFound = true;
					}
				}
			}
		}
	}
	return std::vector<unsigned char>(std::begin(temp), std::end(temp));
}
float Tile::GetHeight(float index, int groundMode, bool smooth) {
	index = std::min(index, 15.f);
	index = std::max(index, 0.f);
	float result;
	if (smooth) {
		float fraction = index - std::floor(index);
		if (groundMode == 0)
			result = Lerp(heightArrays.down[std::floor(index)], heightArrays.down[std::ceil(index)], fraction);
		else if (groundMode == 3)
			result = Lerp(heightArrays.right[std::floor(index)], heightArrays.right[std::ceil(index)], fraction);
		else if (groundMode == 2)
			result = Lerp(heightArrays.up[std::floor(index)], heightArrays.up[std::ceil(index)], fraction);
		else if (groundMode == 1)
			result = Lerp(heightArrays.left[std::floor(index)], heightArrays.left[std::ceil(index)], fraction);
		else
			throw std::invalid_argument("GameTile::GetHeight(): GroundMode out of range. (" + std::to_string(groundMode) + ")");
	}
	else {
		if (groundMode == 0)
			result = heightArrays.down[(int)index];
		else if (groundMode == 3)
			result = heightArrays.right[(int)index];
		else if (groundMode == 2)
			result = heightArrays.up[(int)index];
		else if (groundMode == 1)
			result = heightArrays.left[(int)index];
		else 
			throw std::invalid_argument("GameTile::GetHeight(): GroundMode out of range. (" + std::to_string(groundMode) + ")");
	}
	return result;
}
std::vector<sf::Uint8> *Tile::GetHeightArray(int groundMode) {
	if (groundMode == 0)
		return &this->heightArrays.down;
	else if (groundMode == 3)
		return &this->heightArrays.right;
	else if (groundMode == 2)
		return &this->heightArrays.up;
	else if (groundMode == 1)
		return &this->heightArrays.left;
	else {
		throw std::invalid_argument("GameTile::GetHeightArray(): GroundMode out of range. (" + std::to_string(groundMode) + ")");
	}
}
void Tile::SetHeightArray(short *heightArray, int groundMode) { return; }
sf::Texture* Tile::GetTexture() { return &this->txts->at(texturename); }
sf::IntRect Tile::GetRect() { return sf::IntRect(sf::Vector2i(this->textureRectIndex) * 16, sf::Vector2i(16, 16)); }
void Tile::SetRect(sf::Vector2i index) { return; }
float Tile::GetAngle(int groundMode) {
	bool inverted = false;
	bool upsidedown = false;
	int smallest = heightArrays.down[0];
	int largest = heightArrays.down[0];
	int smallestX = 0;
	int largestX = 0;
	double a = 0;
	if (groundMode == 0) {

		for (int i = 0; i < 16; i++) {

			if ((heightArrays.down[i] <= smallest || smallest == 0) && heightArrays.down[i] != 0) {
				smallest = heightArrays.down[i];
				smallestX = i;
			}
			else if ((heightArrays.down[i] >= largest || smallest == 16) && heightArrays.down[i] != 16) {
				largest = heightArrays.down[i];
				largestX = i;
			}

		}
		a = atan2((smallest - largest), abs(smallestX - largestX));
		if (smallestX > largestX && smallest != largest) inverted = true;
	}
	else if (groundMode == 3) { //Right Wall
		smallest = heightArrays.right[0];
		largest = heightArrays.right[0];
		for (int i = 0; i < 16; i++) {

			if ((heightArrays.right[i] <= smallest || smallest == 0) && heightArrays.right[i] != 0) {
				smallest = heightArrays.right[i];
				smallestX = i;
			}
			else if (heightArrays.right[i] >= largest) {
				largest = heightArrays.right[i];
				largestX = i;
			}
		}
		float width = abs(largest - smallest);
		if (smallestX > largestX) {
			width = -width;
			inverted = true;
		}
		a = atan2((smallestX - largestX), width);
		if (smallest == largest) 
			return 270;
	}
	else if (groundMode == 2) {
		smallest = heightArrays.up[0];
		largest = heightArrays.up[0];
		upsidedown = true;
		for (int i = 0; i < 16; i++) {

			if ((heightArrays.up[i] <= smallest || smallest == 0) && heightArrays.up[i] != 0) {
				smallest = heightArrays.up[i];
				smallestX = i;
			}
			else if (heightArrays.up[i] >= largest) {
				largest = heightArrays.up[i];
				largestX = i;
			}

		}

		if (smallestX < largestX && smallest != largest) inverted = true;
		a = atan2((largest - smallest), abs(smallestX - largestX));

	}
	else if (groundMode == 1) {
		inverted = true;
		smallest = heightArrays.left[0];
		largest = heightArrays.left[0];
		for (int i = 0; i < 16; i++) {

			if ((heightArrays.left[i] <= smallest || smallest == 0) && heightArrays.left[i] != 0) {
				smallest = heightArrays.left[i];
				smallestX = i;
			}
			else if (heightArrays.left[i] >= largest) {
				largest = heightArrays.left[i];
				largestX = i;
			}
		}
		float width = abs(largest - smallest);
		if (smallestX > largestX) {
			width = -width;
			inverted = false;
		}
		a = atan2((smallestX - largestX), width);
		if (smallest == largest)
			return 90;
	}

	if (a < 0) a += 2 * 3.14159;
	a *= 180.0 / 3.14159;
	if (upsidedown)
		a = 180 - a;
	if (inverted)
		return std::ceil(360 - a);
	else
		return std::ceil(a);
}

sf::RectangleShape Tile::floatRectToRect(sf::FloatRect rect) {
	sf::RectangleShape result(sf::Vector2f(rect.width, rect.height));
	result.setPosition(sf::Vector2f(rect.left, rect.top));
	return result;
}
TextureRefInfo Tile::GetRefInfo() {
	return TextureRefInfo(this->texturename, this->textureRectIndex.x, this->textureRectIndex.y);
}
void Tile::SetTileDataFilename(std::vector<std::vector<Tile>>& td, std::string name) {
	for (auto& x : td) {
		for (auto& tile : x) {
			tile.texturename = name;
		}
	}
}
void Tile::SetTxtsPointer(std::vector<std::vector<Tile>>& td, Room *currentRoom) {
	for (auto& x : td) {
		for (auto& tile : x) {
			tile.txts = &currentRoom->textures;
		}
	}
}