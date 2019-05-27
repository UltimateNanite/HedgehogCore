#pragma once
#include "Utilities.h"
#include "Includes.h"


class SpriteSheet {
private:

	std::vector<std::vector<sf::IntRect>> currentSprite;
public:
	int width;
	int height;
	int xIndex;
	int yIndex;
	int prevYIndex;
	float animationtimer = 0;
	float frameduration;
	sf::Sprite sheetS;
	sf::Vector2f origin = sf::Vector2f(30, 46);
	sf::Color fillColor;
	sf::Texture sheetT;
	SpriteSheet(std::string texturepath, int widthin, int heightin);
	SpriteSheet();
	sf::IntRect GetRect();
	sf::IntRect UpdateRect(int x, int y);
	void SetFrame(int xIndex, int yIndex);
};