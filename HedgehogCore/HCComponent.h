#pragma once
#include "Includes.h"
#include <SFML/OpenGL.hpp>
class Room;
struct Player;
class HCObject;
class HCComponent: public sf::Drawable
{
protected:
	HCObject* parent;
	Room* room;
public:
	HCComponent(HCObject *parent, Room* room) : parent(parent), room(room) {};
	~HCComponent();

	virtual void			Update(sf::Time dt)												= 0;
	virtual void			Update(sf::Time dt, Player &player)								= 0;
	virtual HCComponent*	clone()													const	= 0;
	virtual void			im_draw()														= 0;
	virtual void			draw(sf::RenderTarget& target, sf::RenderStates states) const	= 0;

	//operator overloads for saving/loading
	virtual std::ofstream& operator<<(std::ofstream& ofs) = 0;
	virtual std::ifstream& operator>>(std::ifstream& ifs) = 0;
};

class Renderer3D : public HCComponent {
private:
	sf::Vector3f offset = { 0.f, 0.f, -200.f };
	std::vector<GLfloat> points = 
	{
		// positions    // colors (r, g, b, a)
		-50, -50, -50,  0, 0, 1, 1,
		-50,  50, -50,  0, 0, 1, 1,
		-50, -50,  50,  0, 0, 1, 1,
		-50, -50,  50,  0, 0, 1, 1,
		-50,  50, -50,  0, 0, 1, 1,
		-50,  50,  50,  0, 0, 1, 1,

		50, -50, -50,  0, 1, 0, 1,
		50,  50, -50,  0, 1, 0, 1,
		50, -50,  50,  0, 1, 0, 1,
		50, -50,  50,  0, 1, 0, 1,
		50,  50, -50,  0, 1, 0, 1,
		50,  50,  50,  0, 1, 0, 1,

		-50, -50, -50,  1, 0, 0, 1,
		50, -50, -50,  1, 0, 0, 1,
		-50, -50,  50,  1, 0, 0, 1,
		-50, -50,  50,  1, 0, 0, 1,
		50, -50, -50,  1, 0, 0, 1,
		50, -50,  50,  1, 0, 0, 1,

		-50,  50, -50,  0, 1, 1, 1,
		50,  50, -50,  0, 1, 1, 1,
		-50,  50,  50,  0, 1, 1, 1,
		-50,  50,  50,  0, 1, 1, 1,
		50,  50, -50,  0, 1, 1, 1,
		50,  50,  50,  0, 1, 1, 1,

		-50, -50, -50,  1, 0, 1, 1,
		50, -50, -50,  1, 0, 1, 1,
		-50,  50, -50,  1, 0, 1, 1,
		-50,  50, -50,  1, 0, 1, 1,
		50, -50, -50,  1, 0, 1, 1,
		50,  50, -50,  1, 0, 1, 1,

		-50, -50,  50,  1, 1, 0, 1,
		50, -50,  50,  1, 1, 0, 1,
		-50,  50,  50,  1, 1, 0, 1,
		-50,  50,  50,  1, 1, 0, 1,
		50, -50,  50,  1, 1, 0, 1,
		50,  50,  50,  1, 1, 0, 1,
	};
	sf::Time dt;
public:
	Renderer3D* clone() const override { return new Renderer3D(*this); }
	Renderer3D(HCObject *parent, Room* room);
	virtual void Update(sf::Time dt);
	void Update(sf::Time dt, Player& player) override;
	void im_draw() override;
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
	virtual std::ofstream& operator<<(std::ofstream& ofs);
	virtual std::ifstream& operator>>(std::ifstream& ifs);
};
//Required for boost::ptr_vector
HCComponent* new_clone(HCComponent const& other);

class SheetAnimator;
class SpriteRenderer : public HCComponent {
private:
	sf::Texture txt;
	std::string texturename;
	bool txt_valid = false;
	static class WrapMode {
	public:
		static const int Stretch = 0;
		static const int Crop = 1;
		static const int CropRepeat = 2;
	};
	int wrapMode;
	sf::IntRect textureRect;
public:
	using HCComponent::HCComponent;
	SpriteRenderer* clone() const override { return new SpriteRenderer(*this); }
	virtual void Update(sf::Time dt);
	virtual void Update(sf::Time dt, Player& player);
	void im_draw() override;
	void draw(sf::RenderTarget& target, sf::RenderStates states) const;
	virtual std::ofstream& operator<<(std::ofstream& ofs);
	virtual std::ifstream& operator>>(std::ifstream& ifs);

	friend class SheetAnimator;
}; 

class ShapeRenderer : public HCComponent {
private:
	sf::Shape *shape;
public:
	using HCComponent::HCComponent;
	ShapeRenderer* clone() const override { return new ShapeRenderer(*this); }
	virtual void Update(sf::Time dt);
	virtual void Update(sf::Time dt, Player& player);
	void im_draw() override;
	void draw(sf::RenderTarget& target, sf::RenderStates states) const;
	virtual std::ofstream& operator<<(std::ofstream& ofs);
	virtual std::ifstream& operator>>(std::ifstream& ifs);
	
	~ShapeRenderer() {
		if (shape)
			delete shape;
	}
};

class ObjectCollidable : public HCComponent, public HCCollidable {
	//TODO: Extend interface
	

	// Inherited via HCCollidable
	virtual bool CheckCollision(sf::FloatRect collider, PlayerState state) override;

	virtual short GetHeight(sf::Vector2f position, int groundMode) override;

};


class SheetAnimator : public HCComponent {
private:
	sf::IntRect sheetSize;
	sf::Vector2f index;
	
	struct Animation {
		std::vector<sf::Vector2f> indexes;
	};
	std::map<std::string, Animation> data;
	SpriteRenderer* renderer;
public:
	SheetAnimator* clone() const override { return new SheetAnimator(*this); }
	using HCComponent::HCComponent;
	virtual void Update(sf::Time dt);
	void Update(sf::Time dt, Player& player) override;
	void im_draw() override;
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
	virtual std::ofstream& operator<<(std::ofstream& ofs);
	virtual std::ifstream& operator>>(std::ifstream& ifs);
};


//Derived preset
/*
class DerivedComponent : public HCComponent {
private:

public:
	DerivedComponent* clone() const override { return new DerivedComponent(*this); }
	using HCComponent::HCComponent;
	virtual void Update(sf::Time dt);
	void Update(sf::Time dt, Player& player) override;
	void im_draw() override;
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
};

*/