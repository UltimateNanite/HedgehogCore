#include "stdafx.h"
#include "ParallaxLayer.h"


ParallaxLayer::ParallaxLayer()
{
}

ParallaxLayer::ParallaxLayer(sf::Vector2f scrollMultiplier, sf::Texture* image, std::string texturename, bool hLoop, bool yLoop, sf::Vector2f position) {
	this->scrollMultiplier = scrollMultiplier;
	this->texturename = texturename;
	this->image = image;
	this->hLoop = hLoop;
	this->yLoop = yLoop;
	this->position = sf::Vector2f(0,0);
	this->origin = position;
}
ParallaxLayer::~ParallaxLayer()
{
}
