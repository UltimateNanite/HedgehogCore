#pragma once
#include "Includes.h"
#include <SFML/OpenGL.hpp>
#include "Structs.h"
class Room;
struct Player;
class HCObject;
class HCComponent: public sf::Drawable
{
protected:
	HCObject* parent;
	Room* room;
	virtual std::ofstream& filestream_out(std::ofstream& ofs) const = 0;
	virtual std::ifstream& filestream_in(std::ifstream& ifs) = 0;
public:
	HCComponent(HCObject *parent, Room* room) : parent(parent), room(room) {};
	HCComponent() : parent(nullptr), room(nullptr) {};
	~HCComponent();

	//Virtual functions
	virtual void			Update(sf::Time dt)												= 0;
	virtual void			Update(sf::Time dt, Player &player)								= 0;
	virtual void			im_draw()														= 0;
	virtual void			draw(sf::RenderTarget& target, sf::RenderStates states) const	= 0;
	virtual HCComponent*	clone()													const	= 0;

	static HCComponent* create(Room* room, HCObject* parent) { return nullptr; }

	//operator overloads for saving/loading

	friend std::ofstream& operator<<(std::ofstream& ofs, const HCComponent& comp);
	friend std::ifstream& operator>>(std::ifstream& ifs, HCComponent& comp);
	friend class HCCompFactory;
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
	using HCComponent::HCComponent;
	Renderer3D* clone() const override { return new Renderer3D(*this); }
	Renderer3D(HCObject *parent, Room* room);
	virtual void Update(sf::Time dt);
	void Update(sf::Time dt, Player& player) override;
	void im_draw() override;
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
	virtual std::ofstream& filestream_out(std::ofstream& ofs) const;
	virtual std::ifstream& filestream_in(std::ifstream& ifs);

	static HCComponent* create(Room* room, HCObject* parent);


	static HCComponent* factory_create(std::ifstream& ifs);
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
	virtual std::ofstream& filestream_out(std::ofstream& ofs) const;
	virtual std::ifstream& filestream_in(std::ifstream& ifs);

	friend class SheetAnimator;
	static HCComponent* create(Room* room, HCObject* parent);



	static HCComponent* factory_create(std::ifstream& ifs);
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
	virtual std::ofstream& filestream_out(std::ofstream& ofs) const;
	virtual std::ifstream& filestream_in(std::ifstream& ifs);
	
	~ShapeRenderer() {
		if (shape)
			delete shape;
	}
	static HCComponent* create(Room* room, HCObject* parent);


	static HCComponent* factory_create(std::ifstream& ifs);
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
	virtual std::ofstream& filestream_out(std::ofstream& ofs) const;
	virtual std::ifstream& filestream_in(std::ifstream& ifs);
	static HCComponent* create(Room* room, HCObject* parent);


	static HCComponent* factory_create(std::ifstream& ifs);
};


class CollisionHazard : public HCComponent {
private:
public:
	CollisionHazard* clone() const override { return new CollisionHazard(*this); }
	using HCComponent::HCComponent;
	virtual void Update(sf::Time dt);
	void Update(sf::Time dt, Player& player) override;
	void im_draw() override;
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override {};

	virtual std::ofstream& filestream_out(std::ofstream& ofs) const;
	virtual std::ifstream& filestream_in(std::ifstream& ifs);

	static HCComponent* create(Room* room, HCObject* parent);

	static HCComponent* factory_create(std::ifstream& ifs);
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

	virtual std::ofstream& filestream_out(std::ofstream& ofs) const;
	virtual std::ifstream& filestream_in(std::ifstream& ifs);
	static HCComponent* create(Room* room, HCObject* parent);


	static HCComponent* factory_create(std::ifstream& ifs);
};

*/