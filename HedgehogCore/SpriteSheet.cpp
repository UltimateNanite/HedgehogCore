#include "SpriteSheet.h"

SpriteSheet::SpriteSheet(std::string texturepath, int widthin, int heightin) {
	if (!sheetT.loadFromFile(texturepath)) {
		
	}
	sheetS.setTexture(sheetT);
	width = widthin;
	height = heightin;
}
sf::IntRect SpriteSheet::GetRect() {
	return sf::IntRect(xIndex * width + 1, yIndex * height + 2, width, height - 2);
}
sf::IntRect SpriteSheet::UpdateRect(int x, int y) {
	this->xIndex = x;
	this->yIndex = y;
	this->sheetS.setColor(this->fillColor);
	return sf::IntRect(x * width + 1, y * height + 2, width, height - 2);

}
void SpriteSheet::SetFrame(int xIndex, int yIndex) {
	this->xIndex = xIndex;
	this->yIndex = yIndex;
}
SpriteSheet::SpriteSheet() {

}