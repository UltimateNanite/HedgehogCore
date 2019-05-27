#pragma once
#include "Includes.h"
class ParallaxLayer
{
public:
	sf::Texture *image;
	std::string texturename;
	sf::Vector2f scrollMultiplier;
	sf::Vector2f constScrollRate;
	bool hLoop;
	bool yLoop;
	sf::Vector2f position;
	sf::Vector2f origin = sf::Vector2f(0,0);
	ParallaxLayer();
	ParallaxLayer(sf::Vector2f scrollMultiplier, sf::Texture *image, std::string texturename, bool hLoop = false, bool yLoop = false, sf::Vector2f position = sf::Vector2f(0,0));
	~ParallaxLayer();
};

