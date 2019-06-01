#pragma once
#ifndef STRUCTS_H
#define STRUCTS_H
#include "SpriteSheet.h"
#include "Tile.h"
class Tile;
class SpriteSheet;
class AssetSelector;
/*
class EditorSelectable {
public:
	virtual sf::FloatRect getOutline() = 0;
	virtual void imItemsDraw(MonoDomain *domain, Project &currentProject, Room *currentRoom, std::map<std::string, AssetSelector> &assetselectors) = 0;
};*/

struct GlobalGameState {
	bool running;
	float framerate = 60;
	int updatesPerFrame = 4;
};

struct FootSensorData {
	float leftFootHeight;
	float rightFootHeight;
};
struct QuadFootSensorData {
	FootSensorData down;
	FootSensorData right;
	FootSensorData up;
	FootSensorData left;
};
enum Direction {
	None = 0,
	Left = -1,
	Right = 1
};

struct PlayerState {
	Direction pushState;
	int layer;
	Direction lastDirection;
	sf::Vector2f position;
	sf::Vector2f speed;
	double groundSpeed;
	float angle;
	int feetWidth = 9;
	int groundMode;
	int offsetFromGround = 16;
	int horizontalLockTimer;
	bool airborne;
	bool jumping;
	bool crouched;
	bool rolling;
	bool spindashing;
	bool dropdashing;
	bool dead;
	uint16_t rings;
	uint8_t lives;
	float spindashWindup;
};
struct TextureRefInfo {
	std::string name;
	sf::Vector2i textureRectIndex;
	TextureRefInfo(std::string texturename, int x, int y) : textureRectIndex({ x, y }), name(texturename) {}
};
struct TileCollisionTypes {
	static const int Solid = 1 << 0;
	static const int Jumpthrough = 1 << 1;
	static const int Intangible = 1 << 2;
};


class HCCollidable {
public:
	virtual bool CheckCollision(sf::FloatRect collider, PlayerState state) = 0;
	virtual short GetHeight(sf::Vector2f position, int groundMode) = 0;
};


struct WorldTile: public HCCollidable {
	sf::Vector2<unsigned short> chunk;
	short collisiontype = TileCollisionTypes::Solid;
	sf::Vector2<unsigned short> inChunkPos;
	int layer;
	Tile *tile;
	virtual bool CheckCollision(sf::FloatRect collider, PlayerState state) override  {
		if (collisiontype == TileCollisionTypes::Jumpthrough && state.speed.y < 0) 
			return false;
		if (collisiontype == TileCollisionTypes::Intangible) 
			return false;
		sf::FloatRect thisCol(this->GetPosition(), sf::Vector2f(16, 16));
		return thisCol.intersects(collider);
	}

	virtual short GetHeight(sf::Vector2f position, int groundMode) override {
		sf::Vector2i localPos(position - this->GetPosition());
		if (!tile) return 0;
		switch (groundMode) {
		case 0:
			return tile->GetHeight(localPos.x, groundMode);
		case 3:
			return tile->GetHeight(localPos.y, groundMode);
		case 2:
			return tile->GetHeight(localPos.x, groundMode);
		case 1:
			return tile->GetHeight(localPos.y, groundMode);
		default:
			yeet std::exception("GetHeight(): groundMode out of range.");
		}
	}


	sf::Vector2f GetPosition() const { return sf::Vector2f(sf::Vector2i(this->inChunkPos) * 16 + sf::Vector2i(chunk) * 256); }
	void SetPosition(sf::Vector2i position) { this->chunk = sf::Vector2<unsigned short>(position / 16); this->inChunkPos = sf::Vector2<unsigned short>(std::fmod(position.x, 16), std::fmod(position.y, 16)); }
	sf::VertexArray draw(sf::RenderTarget &target, sf::Color multCol = sf::Color::White) const {
		if (!tile) 
			return sf::VertexArray(); 
		
		return tile->draw(sf::Vector2i(this->GetPosition()), target, multCol); 

	}
	
	bool operator== (WorldTile &rhs);
	sf::FloatRect getOutline() {
		return sf::FloatRect(GetPosition(), { 16.f, 16.f });
	}
};

struct PlayerColliderSet {
	PlayerColliderSet() : footSensorHeight(14), footSensorRecline(8) {}
	int footSensorHeight;
	int footSensorRecline;
	WorldTile *leftFootTile;
	WorldTile *rightFootTile;
	WorldTile *leftPushTile;
	WorldTile *rightPushTile;
	sf::FloatRect leftCeilCol;
	sf::FloatRect rightCeilCol;
	sf::FloatRect leftFootCol;
	sf::FloatRect rightFootCol;
	sf::FloatRect pushCol;
	QuadFootSensorData footData;
	void Update(PlayerState &state, int updatesPerFrame) {
		if (state.groundMode == 0) {
			pushCol.width = 20;
			pushCol.height = 1;

			leftFootCol.height = footSensorHeight + footSensorRecline;
			leftFootCol.width = 1;
			leftFootCol.left = state.position.x - state.feetWidth;

			rightFootCol.height = footSensorHeight + footSensorRecline;
			rightFootCol.width = 1;
			rightFootCol.left = state.position.x + state.feetWidth - 1;

			leftCeilCol.height = -footSensorHeight;
			leftCeilCol.width = 1;
			leftCeilCol.left = state.position.x - state.feetWidth;
			leftCeilCol.top = state.position.y;

			rightCeilCol.height = -footSensorHeight;
			rightCeilCol.width = 1;
			rightCeilCol.left = state.position.x + state.feetWidth - 1;
			rightCeilCol.top = state.position.y;
			if (state.airborne) {
				leftFootCol.top = state.position.y - footSensorHeight + state.offsetFromGround;
				rightFootCol.top = state.position.y - footSensorHeight + state.offsetFromGround;
				//leftFootCol.height = footSensorHeight;
				//rightFootCol.height = footSensorHeight;
				leftFootCol.height = state.speed.y;
				rightFootCol.height = state.speed.y;
				pushCol.width += std::abs(state.speed.x) * 2;
			}
			else {
				leftFootCol.top = state.position.y - footSensorRecline + state.offsetFromGround;
				rightFootCol.top = state.position.y - footSensorRecline + state.offsetFromGround;
			}

			if (state.horizontalLockTimer > 0 && !state.airborne)
				state.horizontalLockTimer-= 1 / (float)updatesPerFrame;

			if (state.angle == 0 && !state.airborne)
				pushCol.top = state.position.y + 8;
			else
				pushCol.top = state.position.y;// - std::max(std::abs(state.groundSpeed), 6.) - 4;

			pushCol.left = state.position.x - pushCol.width / 2;
		}
		else if (state.groundMode == 3) {
			pushCol.width = 1;
			pushCol.height = 20;

			pushCol.top = state.position.y - pushCol.height / 2;
			pushCol.left = state.position.x - state.feetWidth;

			leftFootCol.height = 1;
			leftFootCol.width = footSensorHeight + footSensorRecline;
			leftFootCol.left = state.position.x - footSensorRecline;
			leftFootCol.top = state.position.y + state.feetWidth;

			rightFootCol.height = 1;
			rightFootCol.width = footSensorHeight + footSensorRecline;
			rightFootCol.left = state.position.x - footSensorRecline;
			rightFootCol.top = state.position.y - state.feetWidth;

			if (state.airborne) {
				leftFootCol.left = state.position.x - footSensorHeight - footSensorRecline + state.offsetFromGround;
				rightFootCol.left = state.position.x - footSensorHeight - footSensorRecline + state.offsetFromGround;
			}
			else {
				leftFootCol.left = state.position.x - footSensorRecline + state.offsetFromGround;
				rightFootCol.left = state.position.x - footSensorRecline + state.offsetFromGround;
			}
		}
		else if (state.groundMode == 2) {

			pushCol.width = 20;
			pushCol.height = 1;

			leftFootCol.height = footSensorHeight + footSensorRecline;
			leftFootCol.width = 1;
			leftFootCol.left = state.position.x + state.feetWidth;

			rightFootCol.height = footSensorHeight + footSensorRecline;
			rightFootCol.width = 1;
			rightFootCol.left = state.position.x - state.feetWidth;
			if (state.airborne) {
				leftFootCol.top = state.position.y - footSensorHeight - footSensorRecline + state.offsetFromGround;
				rightFootCol.top = state.position.y - footSensorHeight - footSensorRecline + state.offsetFromGround;
			}
			else {
				leftFootCol.top = state.position.y - footSensorHeight - state.offsetFromGround;
				rightFootCol.top = state.position.y - footSensorHeight - state.offsetFromGround;
			}


			if (state.angle == 0)
				pushCol.top = state.position.y + 6;
			else
				pushCol.top = state.position.y + 2;
			pushCol.left = state.position.x - pushCol.width / 2;
		}
		else if (state.groundMode == 1) {
			pushCol.width = 1;
			pushCol.height = 20;

			pushCol.top = state.position.y - pushCol.height / 2;
			pushCol.left = state.position.x + state.feetWidth;

			leftFootCol.height = 1;
			leftFootCol.width = -footSensorHeight - footSensorRecline;
			leftFootCol.left = state.position.x + footSensorRecline;
			leftFootCol.top = state.position.y - state.feetWidth;

			rightFootCol.height = 1;
			rightFootCol.width = -footSensorHeight - footSensorRecline;
			rightFootCol.left = state.position.x + footSensorRecline;
			rightFootCol.top = state.position.y + state.feetWidth;

			if (state.airborne) {
				leftFootCol.left = state.position.x - footSensorHeight - footSensorRecline + state.offsetFromGround;
				rightFootCol.left = state.position.x - footSensorHeight - footSensorRecline + state.offsetFromGround;
			}
			else {
				leftFootCol.left = state.position.x + footSensorRecline - state.offsetFromGround;
				rightFootCol.left = state.position.x + footSensorRecline - state.offsetFromGround;
			}
		}
	}
};
struct PhysicsConsts {
	float acc = 0.046875; //Acceleration
	float frc = acc;      //Friction when xspd != 0 and not holding anything.
	float dec = 0.5;      //Deceleration when holding opposite direction
	float grv = 0.21875f;
	float jmp = -6.5f - grv;
};

struct Player : boost::noncopyable {
	PlayerColliderSet colliders;
	PlayerState state;
	SpriteSheet sprite;
	PhysicsConsts physics;
	Player() {
		this->state.offsetFromGround = 16;
		this->state = PlayerState();
		this->colliders = PlayerColliderSet();
		this->sprite = SpriteSheet();
		this->physics = PhysicsConsts();
	}

	void hurt(Room *room) {
		if (state.rings == 0) {
			state.dead = true;
		}
		else {
			uint16_t rings = state.rings;
			state.rings = 0;
			//TODO: Rings spawning
		}
	}
};

struct Camera {
	sf::Vector2f position;
	sf::Vector2f offset;
	float zoomFactor = 1.f;
};


class CollisionTypes {
public:
	const static int FootSensor = 1;
	const static int WallSensor = 2;
};



namespace sf {
	//WIP
	class FastSprite : Drawable, Transformable {
	private:
		Vector2f position;
		IntRect textureRect;
		Texture *texture;

		virtual void draw(RenderTarget& target, RenderStates states) const {

		}

	public:
		const Vector2f& getPosition() const {
			return position;
		}
		void move(float offsetX, float offsetY) {
			position += {offsetX, offsetY};
		}
		void move(const Vector2f& offset) {
			position += offset;
		}
	};

}


WorldTile *GetTileAtGlobalIndex(sf::Vector2i index, int layer, Room *currentRoom);
#endif