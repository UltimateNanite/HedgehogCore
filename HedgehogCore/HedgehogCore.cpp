// HedgehogCreator.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Includes.h"
#include "Structs.h"
#include "Room.h"

#include "HCObject.h"
#include "ParallaxLayer.h"
#include "Project.h"

#include "tinyfiledialog.h"
#include <filesystem>
#include "SimpleIni.h"
namespace fs = std::filesystem;

#define chunkSizePx 256
#define TILE_HEIGHT 16

#define EDITOR_ROOM 0
#define EDITOR_PARALLAX 1
#define EDITOR_ANIMATION 2
#define EDITOR_HEIGHTMAP 3

#define CHUNK_UNKNOWN 0
#define CHUNK_EMPTY 1
#define CHUNK_NONEMPTY 2
byte emptychunks[64][8];

#define GROUNDMODE_FLOOR 0
#define GROUNDMODE_RIGHTWALL 3
#define GROUNDMODE_CEILING 2
#define GROUNDMODE_LEFTWALL 1


#ifndef _DEBUG
#define _RUNTIMEONLY
#endif



int debugFrameCounter = 0;


int debugdrawcalls = 0;

//Prototypes
void FixedUpdate(GlobalGameState& gamestate, Player& player, sf::Time dt);
void Draw(sf::RenderWindow& target, Player& player);
void EventHandling(sf::Event event, GlobalGameState& state, Player& player);

//Global BC why not

Camera camera;
bool scriptpushcontrol;


//bool editing = false;
GlobalGameState state;

Room currentRoom;
//Project currentProject;

ParallaxLayer* selectedPL;

std::vector<std::string> recentprojects;
sf::RenderTexture gameWindow;
sf::Sprite gameDraw;
sf::Texture referenceimage;

const sf::Vector2i gameWindowSize(424, 240);
//debug
float lastXspd;
float lastYspd;
Player* CS_player;

float fps;
sf::Font consolas;

float drawT;
float updT;
float otherT;
namespace profilerTimes {
	using namespace sf;
	static Time updateTime;
	static Time drawTime;
	static Time other;
	static Time total;
}
using namespace boost::container;

template<class T>
void ref_bound(T lowerBound, T& real, T top) {
	real = std::min(std::max(real, lowerBound), top);
}

template<class T>
T bound(T lowerBound, T real, T top) {
	return std::min(std::max(real, lowerBound), top);
}



template<class T> inline T operator~ (T a) { return (T)~(int)a; }
template<class T> inline T operator| (T a, T b) { return (T)((int)a | (int)b); }
template<class T> inline T operator|| (T a, T b) { return (T)((int)a | (int)b); }
template<class T> inline T operator& (T a, T b) { return (T)((int)a & (int)b); }
template<class T> inline T operator^ (T a, T b) { return (T)((int)a ^ (int)b); }
template<class T> inline T& operator|= (T & a, T b) { return (T&)((int&)a |= (int)b); }
template<class T> inline T& operator&= (T & a, T b) { return (T&)((int&)a &= (int)b); }
template<class T> inline T& operator^= (T & a, T b) { return (T&)((int&)a ^= (int)b); }



void RoomStart(Room* currentRoom) {
	/*currentRoom.roomMusic.stop();
	currentRoom.roomMusic.openFromFile(GetCurrentProjectPath() + "/Assets/" + currentRoom.songfilename);
	currentRoom..play();
	currentRoom.roomMusic.setLoop(true);
	currentRoom.roomMusic.setLoopPoints(Music::TimeSpan(seconds(14.425f), seconds(roomMusic.getDuration().asSeconds() - 1.6f))); //Slow*/
}



constexpr int InverseGroundMode(int groundMode) {
	switch (groundMode) {
	case 0:
		return 2;
	case 3:
		return 1;
	case 2:
		return 0;
	case 1:
		return 3;
	}
	return -1;
}

bool CollisionHandling(sf::FloatRect col, WorldTile * currentTile, WorldTile * *assignedTilePtr, Player & player, bool& hasCollided, bool floor = true) {
	bool recognized = (player.state.speed.y >= 0 || !floor || !player.state.airborne);
	if (*assignedTilePtr == nullptr && (recognized || !player.state.airborne))
		* assignedTilePtr = currentTile;

	float tempHeight = currentTile->GetHeight(sf::Vector2f(col.left, col.top), std::abs(player.state.groundMode));

	bool validHeight = ((
		(player.state.groundMode == GROUNDMODE_FLOOR || player.state.groundMode == GROUNDMODE_RIGHTWALL || player.state.groundMode == GROUNDMODE_LEFTWALL) && tempHeight != 0) ||
		player.state.groundMode == GROUNDMODE_CEILING && tempHeight != 0);
	if (currentTile->tile) {
		if (player.state.groundMode == GROUNDMODE_FLOOR) {
			int localPos = col.left - currentTile->GetPosition().x;
			validHeight = currentTile->GetPosition().y + currentTile->tile->GetHeight(localPos, InverseGroundMode(GROUNDMODE_FLOOR), false) >= player.colliders.pushCol.top || !floor;
		}
		else if (player.state.groundMode == GROUNDMODE_RIGHTWALL) {
			//int localPos = player.state.position.x;
		}
		else if (player.state.groundMode == GROUNDMODE_CEILING) {

		}
		else if (player.state.groundMode == GROUNDMODE_LEFTWALL) {

		}
	}
	//int localPos = player.colliders.pushCol.left + player.colliders.pushCol.width - currentTile->GetPosition().x;
	//if (currentTile->tile &&
	//	currentTile->GetPosition().y + 16 - currentTile->tile->GetHeightArray(0)->at(localPos) <= player.colliders.pushCol.top &&
	//	currentTile->GetPosition().y + currentTile->tile->GetHeightArray(2)->at(localPos) > player.colliders.pushCol.top) {
	if (validHeight && (recognized || !player.state.airborne)) {

		if ((player.state.groundMode == GROUNDMODE_FLOOR &&
			currentTile->GetPosition().y <= (*assignedTilePtr)->GetPosition().y) ||
			(player.state.groundMode == GROUNDMODE_RIGHTWALL &&
				currentTile->GetPosition().x <= (*assignedTilePtr)->GetPosition().x) ||
				(player.state.groundMode == GROUNDMODE_CEILING &&
					currentTile->GetPosition().y >= (*assignedTilePtr)->GetPosition().y) ||
					(player.state.groundMode == GROUNDMODE_LEFTWALL &&
						currentTile->GetPosition().x >= (*assignedTilePtr)->GetPosition().x))
		{

			hasCollided = true;
			*assignedTilePtr = currentTile;
			return true;
		}
	}
	else if (currentTile == *assignedTilePtr) {
		*assignedTilePtr = nullptr;
	}

	return false;
}

WorldTile* GetTileAtGlobalIndex(sf::Vector2i index, int layer, Room * currentRoom) {
	using namespace sf;
	index.x = std::max(index.x, 0);
	index.y = std::max(index.y, 0);
	Vector2i currentChunk(index.x / 16, index.y / 16);
	Vector2i currentPosInChunk(std::fmod(index.x, 16), std::fmod(index.y, 16));
	return &currentRoom->chunks[layer][currentChunk.x][currentChunk.y][currentPosInChunk.x][currentPosInChunk.y];
}


bool WorldTile::operator== (WorldTile & rhs) {
	return this->GetPosition() == rhs.GetPosition();
}
namespace menu
{
	bool projectWindow = true;
	bool toolsWindow = true;
	bool objectsWindow = true;
	bool playerWindow = true;
}

class DebugInfoLevels {
public:
	static const int NONE = 0;
	static const int MIN = 1;
	static const int MAX = 2;
};
int debugInfoLevel = DebugInfoLevels::MAX;

float CalculateGSPD(float floorAngle, PlayerState state) {
	float result = 0;
	//Calculate GSPD
	float internalFloorAngle = floorAngle;
	result = state.speed.x * cos(Rad(state.angle));

	if (floorAngle > 180)
		internalFloorAngle = 360 - floorAngle;

	if (internalFloorAngle < 22.5f)
		result = state.speed.x;
	else if (internalFloorAngle < 45) {
		if (abs(state.speed.x) > state.speed.y)
			result = state.speed.x;
		else
			result = state.speed.y * 0.5 * GetSign(state.speed.x);// *-GetSign(sin(Rad(internalFloorAngle)));
	}
	else if (internalFloorAngle < 90) {
		if (abs(state.speed.x) > state.speed.y)
			result = state.speed.x;
		else
			result = state.speed.y * GetSign(sin(Rad(state.angle)));// *-GetSign(sin(Rad(internalFloorAngle)));
	}
	return result;
}
using namespace sf;
Player yourenotallowedtousethisplayerinstanceexceptinmainwhichiswhyimadethenamesoinconvenientlylong; //Needs to be global for some reason, doesn't initialise properly otherwise
																									 //This and the main function should be the only places "globplayer" are used. Don't treat this as global, pass a Player& to functions instead.


bool playIntro(sf::RenderWindow& window) {
#ifdef _DEBUG
	return true; //Don't play intro if debugging
#endif

	using namespace sf;


	


	int timer = 0;

	window.setFramerateLimit(60);
	while (timer < 60 * 10) {
		timer++;


		Event event;
		while (window.pollEvent(event)) {
			if (event.type == Event::Closed) {
				window.close();
				return false;
			}
		}



		window.clear(Color::Black);

		Text poweredBy("powered by", consolas);

		window.draw(poweredBy);
		window.display();
	}
	return true;

	window.setFramerateLimit(0);
}





const bool needsExternalRuntime = false;
int main(int argc, char* argv[])
{
	std::vector<std::string> args(argv, argv + argc);
	bool startupRoomOverride = false;

	//currentProject = Project();
	//currentProject.GetRoomsPtr()->push_back(Room(&currentProject));

	


#ifdef _RUNTIMEONLY
	FreeConsole();
#endif
	referenceimage.loadFromFile("referenceimage.png");

	//Init OpenGL
	// Make the window the active window for OpenGL calls
	gameWindow.setActive(true);

	// Enable Z-buffer read and write

	glClearDepth(1.f);
	glClearColor(0.f, 0.f, 0.f, 1.f);


	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);

	// Disable lighting
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	// Configure the viewport (the same size as the window)
	glViewport(0, 0, gameWindow.getSize().x, gameWindow.getSize().y);

	//Setup a perspective projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	GLfloat ratio = static_cast<float>(gameWindow.getSize().x) / gameWindow.getSize().y;
	glFrustum(-ratio, ratio, -1.f, 1.f, 1.f, 500.f);
	// Disable normal and color vertex components


	// Make the window no longer the active window for OpenGL calls
	gameWindow.setActive(false);


	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_ALPHA_TEST);



	consolas.loadFromFile("Default\\Fonts\\consola.ttf");
	Player& player = yourenotallowedtousethisplayerinstanceexceptinmainwhichiswhyimadethenamesoinconvenientlylong;
	player.state.position = { 100, 100 };
	player.state.lastDirection = Direction::Right;


	state.running = true;

	if (!gameWindow.create(gameWindowSize.x, gameWindowSize.y)) {
		std::cout << "Failed to initialise gameWindow";
		system("pause");
		return 0;
	}
	//Display
	Vector2<unsigned short> windowRes(gameWindowSize);
	windowRes *= (unsigned short)std::min(VideoMode::getDesktopMode().width / windowRes.x, VideoMode::getDesktopMode().height / windowRes.y);

	ContextSettings settings;
	//Display initialisation
	settings.depthBits = 24;
	settings.stencilBits = 8;
	settings.antialiasingLevel = 8; // Optional


	settings.attributeFlags = ContextSettings::Core;
	RenderWindow window(VideoMode(windowRes.x, windowRes.y), "HedgehogCreator Runtime", Style::Close, settings);



	window.setKeyRepeatEnabled(false); //Disable repeated keypresses




	window.clear(Color::Black);
	Text initText;
	initText.setFont(consolas);
	initText.setCharacterSize(40);

	initText.setFillColor(Color::White);
	initText.setString("Loading...");
	initText.setPosition(Vector2f(windowRes) / 2.f - Vector2f(initText.getGlobalBounds().width, initText.getGlobalBounds().height) / 2.f);
	window.draw(initText);
	window.display();

	currentRoom.name = "New Room";

	for (unsigned int i = 1; i < args.size(); i++) {
		std::string arg = args[i];
		std::cout << arg << std::endl;
		if (arg == "-d:0") {
			debugInfoLevel = DebugInfoLevels::NONE;
		}
		if (arg == "-d:1") {
			debugInfoLevel = DebugInfoLevels::MIN;
		}
		if (arg == "-d:2") {
			debugInfoLevel = DebugInfoLevels::MAX;
		}
		if (arg._Starts_with("-r:")) {
			std::ifstream ifs(arg.substr(3));
			std::cout << "Loading " + arg.substr(3) << std::endl;
			if (!(fs::exists(arg.substr(3)))) {
				throw std::runtime_error("File does not exist.");
			}
			//currentRoom = &currentProject.GetRoomsPtr()->at(0);
			ifs >> currentRoom;
			startupRoomOverride = true;
			ifs.close();
		}
	}

	//if (!playIntro(window)) 
	//	return false;
	
	fs::path dir("startupdata.ini");
	if (fs::exists(dir)) {
		CSimpleIniA ini;
		ini.SetUnicode();
		ini.LoadFile("startupdata.ini");
		const char* title = ini.GetValue("Global", "title", "HedgehogCreator (No game title found)");
		window.setTitle(title);

		int rooms_size = std::stoi(ini.GetValue("Global", "rooms_size", "-1"));
		if (rooms_size < 0) 
			yeet std::exception("No rooms found in startupdata.ini (rooms_size <= 0)");

		for (int i = 0; i < rooms_size; i++) {
			std::string currentRoom = "room" + std::to_string(i);
			std::string path = ini.GetValue(currentRoom.c_str(), "path", "");
			if (path == "")
				yeet std::exception("Cannot read app: startupdata.ini is incorrect.");
			std::ifstream in(path);
			Room result = Room();
			in >> result;
		}
	}
	else if (!startupRoomOverride) {

		std::cout << "startupdata.ini not found, loading default room..\n";

		


		currentRoom.AddTileMap("CPZ.png");

		currentRoom.AddTileMap("GHZ.png");
		currentRoom.AddTileMap("testmap.png");



	


		Texture ghzbg1;
		ghzbg1.loadFromFile("Sprites\\GHZ Test\\ghz_background_1.png");
		currentRoom.textures.insert(std::pair<std::string, sf::Texture>("Sprites\\GHZ Test\\ghz_background_1.png", ghzbg1));
		Texture ghzbg2;
		ghzbg2.loadFromFile("Sprites\\GHZ Test\\ghz_background_2.png");
		currentRoom.textures.insert(std::pair<std::string, sf::Texture>("Sprites\\GHZ Test\\ghz_background_2.png", ghzbg2));
		Texture ghzbg3;
		ghzbg3.loadFromFile("Sprites\\GHZ Test\\ghz_background_3.png");
		currentRoom.textures.insert(std::pair<std::string, sf::Texture>("Sprites\\GHZ Test\\ghz_background_3.png", ghzbg3));
		Texture temp;

		currentRoom.parallaxLayers.push_back(ParallaxLayer(sf::Vector2f(0.5f, 1.f), nullptr, "Sprites\\GHZ Test\\ghz_background_1.png", true, false));
		currentRoom.parallaxLayers.push_back(ParallaxLayer(sf::Vector2f(0.5f, 1.f), nullptr, "Sprites\\GHZ Test\\ghz_background_2.png", true, false));
		currentRoom.parallaxLayers.push_back(ParallaxLayer(sf::Vector2f(0.8f, 1.f), nullptr, "Sprites\\GHZ Test\\ghz_background_3.png", true, false));

		for (int i = 1; i < 13; i++) {
			temp = Texture();
			std::string filename("Sprites\\GHZ Test\\ghz_bg_water_" + std::to_string(i) + ".png");
			temp.loadFromFile(filename);
			currentRoom.textures.insert(std::pair<std::string, sf::Texture>(filename, temp));
			currentRoom.parallaxLayers.push_back(ParallaxLayer({ 0.9f, 1.f }, nullptr, filename, true, false, Vector2f(0, temp.getSize().y * i + 112)));
			currentRoom.parallaxLayers[currentRoom.parallaxLayers.size() - 1].image = &currentRoom.textures[filename];
			currentRoom.parallaxLayers[currentRoom.parallaxLayers.size() - 1].constScrollRate = sf::Vector2f(-i / 26.f, 0.f);
		}

		currentRoom.parallaxLayers[0].image = &currentRoom.textures["Sprites\\GHZ Test\\ghz_background_1.png"];
		currentRoom.parallaxLayers[0].constScrollRate.x = -3;
		currentRoom.parallaxLayers[1].image = &currentRoom.textures["Sprites\\GHZ Test\\ghz_background_2.png"];
		currentRoom.parallaxLayers[1].origin.y = currentRoom.parallaxLayers[0].image->getSize().y;
		currentRoom.parallaxLayers[1].constScrollRate.x = -2;
		currentRoom.parallaxLayers[2].image = &currentRoom.textures["Sprites\\GHZ Test\\ghz_background_3.png"];
		currentRoom.parallaxLayers[2].origin.y = currentRoom.parallaxLayers[0].image->getSize().y + currentRoom.parallaxLayers[1].image->getSize().y;
	}

	CS_player = &player;
	player.sprite = SpriteSheet("Sprites\\Sonic\\sonicSheet.png", 64, 64);

	std::cout << "HedgehogCreator Runtime\n";
	Time dt;
	Clock deltaClock;
	Clock profilerClock;
	double fixedTimer = 0.0;
	float timestep = 1 / fps;

	gameWindow.setActive();
	bool hadFocusLastUpdate = false;
	while (state.running) {
#ifndef _DEBUG
		try {
#endif
			timestep = 1 / state.framerate;
			Event event;
			while (window.pollEvent(event)) {
				if (window.hasFocus())
					EventHandling(event, state, player);
			}
			if (!window.hasFocus())
				continue;


			profilerTimes::other = profilerClock.getElapsedTime();



			gameWindow.clear(Color::Black);

			while (fixedTimer > timestep) {

				
				FixedUpdate(state, player, dt);
				fixedTimer -= timestep;

			}

			profilerTimes::updateTime = profilerClock.getElapsedTime() - profilerTimes::other;

			Draw(window, player);

			profilerTimes::drawTime = profilerClock.getElapsedTime() - profilerTimes::other - profilerTimes::updateTime;
			profilerTimes::total = profilerClock.restart();

			dt = deltaClock.restart();

			if (window.hasFocus()) {

				if (!hadFocusLastUpdate)
					dt = deltaClock.restart(); //Reset clock before resuming

				fixedTimer += dt.asSeconds();

				hadFocusLastUpdate = true;
			}
			else
				hadFocusLastUpdate = false;

			fps = Lerp(fps, 1 / dt.asSeconds(), 0.1f);
#ifndef _DEBUG
		}
		catch (const std::exception & e) {
			window.close();
			SetFocus(GetConsoleWindow());
			std::cout << "\nAn unhandled exception has occurred in HedgehogCreator.";
			std::cout << "\nError: " << e.what();
			std::cout << "\nAttempt to backup project? (y/n)\n > ";
			char choice = '\0';
			std::cin >> choice;
			if (choice == 'y' || choice == 'Y') {
				currentRoom.name = currentRoom.name + "_backup";
				//currentRoom.path = GetOwnFilePath() + "/Projects/" + currentProject.name + "/";
				//currentRoom.Save(currentProject.path);
			}
			system("pause");
			state.running = false;
			return -1;
		}
#endif
	}

	return 0;
}

WorldTile* GetTileAtPosition(sf::Vector2f position, int layer) {
	return GetTileAtGlobalIndex(Vector2i(std::floorf(std::max(position.x / 16.f, 0.f)), std::ceilf(std::max(position.y / 16.f, 0.f))), layer, &currentRoom);
}

std::vector<WorldTile*> GetTilesWithinRange(sf::FloatRect rect, int layer) {
	std::vector<WorldTile*> result;
	for (int x = std::max(rect.left / 16 - 1, 0.f); x < std::min((rect.left + rect.width) / 16 + 1, 16 * 64.f); x++) {
		for (int y = std::max(rect.top / 16 - 1, 0.f); y < std::min((rect.top + rect.height) / 16 + 1, 16 * 8.f); y++) {
			result.push_back(GetTileAtGlobalIndex(sf::Vector2i(x, y), layer, &currentRoom));
		}
	}
	return result;
}
/*std::vector<HCCollidable*> GetTilesWithinRange(sf::FloatRect rect, int layer) {
	std::vector<HCCollidable*> result;
	for (int x = std::max(rect.left / 16 - 1, 0.f); x < std::min((rect.left + rect.width) / 16 + 1, 16 * 64.f); x++) {
		for (int y = std::max(rect.top / 16 - 1, 0.f); y < std::min((rect.top + rect.height) / 16 + 1, 16 * 8.f); y++) {
			result.push_back(GetTileAtGlobalIndex(sf::Vector2i(x, y), layer, &currentRoom));
		}
	}
	return result;
}*/

void FixedUpdate(GlobalGameState & gamestate, Player & player, sf::Time dt) {
	using namespace sf;
	PhysicsConsts& physics = player.physics;
	bool hasAcc = false;
	if (camera.zoomFactor > 1) {
		if (camera.zoomFactor > 1.01f)
			camera.zoomFactor = Lerp(camera.zoomFactor, 1, 0.5f);
		else
			camera.zoomFactor = 1;
	}
	else if (camera.zoomFactor < 1) {
		if (camera.zoomFactor < 0.99f)
			camera.zoomFactor = Lerp(camera.zoomFactor, 1, 0.5f);
		else
			camera.zoomFactor = 1;
	}
	for (int i = 0; i < gamestate.updatesPerFrame; i++) {
		if (!player.state.crouched && !player.state.airborne) {
			if (!player.state.spindashing) {
				player.state.groundSpeed += 0.125 * sin(Rad(player.state.angle)) / (float)gamestate.updatesPerFrame; //Slope factor
			}
			else {
				if (GetSign(player.state.groundSpeed) == GetSign(sin(Rad(player.state.angle)))) {
					player.state.groundSpeed += 0.078125 / (float)gamestate.updatesPerFrame;
				}
				else {
					player.state.groundSpeed += 0.3125 / (float)gamestate.updatesPerFrame;
				}
			}
		}

		//Input & movement
		//REMINDER: Acc, dec and frc are applied to GSPD, not player.state.speed.x.
		if (player.state.spindashWindup < 0)
			player.state.spindashWindup = 0;

		if (std::abs(player.state.spindashWindup) >= 0.125)
			player.state.spindashWindup *= 0.96875 / (float)gamestate.updatesPerFrame;


		if (Keyboard::isKeyPressed(Keyboard::A) && !player.state.crouched && player.state.horizontalLockTimer == 0) {
			player.state.lastDirection = Direction::Left;
			if (player.state.airborne) {
				if (player.state.pushState > -1 && player.state.speed.x > -6) {
					player.state.speed.x -= (physics.acc * 2) / (float)gamestate.updatesPerFrame; //If airborne, multiply acc by 2
					hasAcc = true;
				}
			}
			else {
				if (player.state.pushState != Direction::Left) {
					if (player.state.groundSpeed > 0) {
						player.state.groundSpeed -= physics.dec / (float)gamestate.updatesPerFrame;
						hasAcc = true;
					}
					else if (player.state.groundSpeed > -6 && !player.state.crouched && !player.state.rolling) {
						player.state.groundSpeed -= physics.acc / (float)gamestate.updatesPerFrame;
						hasAcc = true;
					}
				}
			}
		}
		else if (Keyboard::isKeyPressed(Keyboard::D) && !player.state.crouched && player.state.horizontalLockTimer == 0) {
			player.state.lastDirection = Direction::Right;
			if (player.state.airborne) {
				if (player.state.pushState != Direction::Left && player.state.speed.x < 6) {
					player.state.speed.x += (physics.acc * 2) / (float)gamestate.updatesPerFrame;
					hasAcc = true;
				}
			}
			else {
				if (player.state.pushState < 1) {
					if (player.state.groundSpeed < 0) {
						player.state.groundSpeed += physics.dec / (float)gamestate.updatesPerFrame;
						hasAcc = true;
					}
					else if (player.state.groundSpeed < 6 && !player.state.crouched && !player.state.rolling) {
						player.state.groundSpeed += physics.acc / (float)gamestate.updatesPerFrame;
						hasAcc = true;
					}
				}
			}
		}


		if (Keyboard::isKeyPressed(Keyboard::S)) {
			if (!player.state.crouched && !player.state.rolling) {

				if (abs(player.state.groundSpeed) < 0.1f && !player.state.airborne) {
					player.state.crouched = true;
					player.sprite.xIndex = 0;
					player.state.speed.x = 0;
				}
				else if (abs(player.state.groundSpeed) < 1.03125f && !player.state.airborne) {
					player.state.crouched = true;
				}
				else {
					player.state.rolling = true;
				}
			}

		}
		else {
			player.state.crouched = false;
		}


		if (!Keyboard::isKeyPressed(Keyboard::Space)) {
			if (player.state.jumping && player.state.speed.y < -4)
				player.state.speed.y = -4;
			player.state.dropdashing = false;
		}

		if (!player.state.airborne)
			player.state.speed.y = player.state.groundSpeed * sin(Rad(player.state.angle));
		else
			player.state.speed.y += physics.grv / (float)gamestate.updatesPerFrame;




		if (abs(player.state.groundSpeed) < 0.5f && player.state.rolling && player.state.groundMode == 0 && !player.state.airborne) {
			player.state.rolling = false;
			player.state.crouched = true;
		}


		//Bugfix where player would crouch in air/when sliding fast sometimes
		if (abs(player.state.groundSpeed) >= 0.5f && !player.state.rolling && player.state.crouched) {
			player.state.crouched = false;
			player.state.rolling = true;
		}

		if (!Keyboard::isKeyPressed(Keyboard::S) && player.state.spindashing) {
			player.state.spindashing = false;
			player.state.rolling = true;
			player.state.groundSpeed = (8 + (floor(player.state.spindashWindup) / 2.f)) * (int)player.state.lastDirection;
			player.state.spindashWindup = 0;
		}



		if (abs(player.state.groundSpeed) < 2.5 && player.state.groundMode != 0) {
			if (360 - player.state.angle >= 90 && 360 - player.state.angle <= 270) {
				player.state.groundMode = 0;
				player.state.groundSpeed = 0;
				player.state.airborne = true;
			}
			player.state.horizontalLockTimer = 32;
			player.state.crouched = false;
		}



		//Airdrag
		if (player.state.speed.y < 0 && player.state.speed.y > -4 && player.state.airborne)
		{
			if (std::abs(player.state.speed.x) >= 0.125)
				player.state.speed.x *= 0.996875;
		}



		//Sonic animation and drawing

		if ((int)player.state.lastDirection == Direction::None) player.state.lastDirection = Direction::Right;
		player.sprite.sheetS.setScale((int)player.state.lastDirection, 1);


		if ((unsigned int)player.state.layer > currentRoom.chunks.size() - 1)
			player.state.layer = currentRoom.chunks.size() - 1;
		//Drawing layers behind Sonic














		//Collision & Movement


		//Set up Sensors
		player.colliders.Update(player.state, state.updatesPerFrame);

		player.state.position +=
			Vector2f(
				player.state.speed.x / (float)gamestate.updatesPerFrame,
				player.state.speed.y / (float)gamestate.updatesPerFrame);

		player.state.position = Vector2f(std::max(player.state.position.x, 0.f), std::max(player.state.position.y, 0.f));
		player.state.position = Vector2f(std::min(player.state.position.x, currentRoom.chunks[0].size() * (float)chunkSizePx - 1), std::min(player.state.position.y, currentRoom.chunks[0][0].size() * (float)chunkSizePx - 1));
		bool hasCollided = false;



		player.colliders.leftFootTile = nullptr;
		player.colliders.rightFootTile = nullptr;
		player.state.angle = std::abs((int)player.state.angle % 360);






		hasCollided = false;
		std::vector<WorldTile*> leftTiles = GetTilesWithinRange(player.colliders.leftFootCol, player.state.layer);
		for (auto currentTile : leftTiles) {
			if (currentTile->CheckCollision(player.colliders.leftFootCol, player.state))
				CollisionHandling(player.colliders.leftFootCol, currentTile, &player.colliders.leftFootTile, player, hasCollided);
		}
		std::vector<WorldTile*> rightTiles = GetTilesWithinRange(player.colliders.rightFootCol, player.state.layer);
		for (auto currentTile : rightTiles) {
			if (currentTile->CheckCollision(player.colliders.rightFootCol, player.state))
				CollisionHandling(player.colliders.rightFootCol, currentTile, &player.colliders.rightFootTile, player, hasCollided);
		}
		//Collision issues likely to do with incorrect truncating. std::round()?

		Vector2i currentTileWorldIndex = Vector2i(player.state.position) / 16;


		player.colliders.Update(player.state, state.updatesPerFrame);


		Vector2i currentChunkPos(Vector2f((Vector2i(player.state.position) / 16 * 16)) / Vector2f((int)chunkSizePx, chunkSizePx));


		if (player.state.pushState == Direction::Right) {
			if (player.colliders.rightPushTile->tile) {
				if (player.colliders.pushCol.top + 1 < player.colliders.rightPushTile->GetPosition().y
					|| player.colliders.pushCol.top + 1 > player.colliders.rightPushTile->GetPosition().y + TILE_HEIGHT
					|| player.state.speed.x < 0) {
					player.state.pushState = Direction::None;
					player.colliders.rightPushTile = nullptr;
				}
			}
			else if (scriptpushcontrol && player.state.speed.x < 0)
				player.state.pushState = Direction::None;
			else if (!scriptpushcontrol)
				player.state.pushState = Direction::None;
		}
		else if (player.state.pushState == Direction::Left) {
			if (player.colliders.leftPushTile->tile) {
				if (player.colliders.pushCol.top + 1 < player.colliders.leftPushTile->GetPosition().y
					|| player.colliders.pushCol.top + 1 > player.colliders.leftPushTile->GetPosition().y + TILE_HEIGHT
					|| player.state.speed.x > 0) {
					player.state.pushState = Direction::None;
					player.colliders.leftPushTile = nullptr;
				}
			}
			else if (scriptpushcontrol && player.state.speed.x > 0)
				player.state.pushState = Direction::None;
			else if (!scriptpushcontrol)
				player.state.pushState = Direction::None;
		}



		WorldTile* throwAway = nullptr; //Throwaway variable for CollisionHandling() to write to
		bool throwAwayBool = false;
		if (player.state.groundMode == GROUNDMODE_FLOOR) {
			for (auto currentTile : GetTilesWithinRange(player.colliders.leftCeilCol, player.state.layer)) {
				if (currentTile->CheckCollision(player.colliders.leftCeilCol, player.state)) {
					if (CollisionHandling(player.colliders.leftCeilCol, currentTile, &throwAway, player, throwAwayBool, false)) {
						if (player.state.speed.y < 0)
							player.state.position.y = currentTile->GetPosition().y + currentTile->GetHeight(player.state.position, 2) + player.colliders.footSensorHeight;
						hasCollided = false;
						player.state.speed.y = std::max(player.state.speed.y, 0.f);
					}
				}
			}
			for (auto currentTile : GetTilesWithinRange(player.colliders.rightCeilCol, player.state.layer)) {
				if (currentTile->CheckCollision(player.colliders.rightCeilCol, player.state)) {
					if (CollisionHandling(player.colliders.rightCeilCol, currentTile, &throwAway, player, throwAwayBool, false)) {
						if (player.state.speed.y < 0)
							player.state.position.y = currentTile->GetPosition().y + currentTile->GetHeight(player.state.position, 2) + player.colliders.footSensorHeight;
						hasCollided = false;
						player.state.speed.y = std::max(player.state.speed.y, 0.f);
					}
				}
			}

		}



		if (player.colliders.leftFootTile && player.colliders.rightFootTile) {

			//Get floor heights
			float tempHeightD = player.colliders.leftFootTile->GetHeight(Vector2f(player.colliders.leftFootCol.left, player.colliders.leftFootCol.top), 0);
			float tempHeightR = player.colliders.leftFootTile->GetHeight(Vector2f(player.colliders.leftFootCol.left, player.colliders.leftFootCol.top), 3);
			float tempHeightU = player.colliders.leftFootTile->GetHeight(Vector2f(player.colliders.leftFootCol.left, player.colliders.leftFootCol.top), 2);
			float tempHeightL = player.colliders.leftFootTile->GetHeight(Vector2f(player.colliders.leftFootCol.left, player.colliders.leftFootCol.top), 1);
			QuadFootSensorData& fD = player.colliders.footData;
			fD.down.leftFootHeight = player.colliders.leftFootTile->GetPosition().y + 16 - tempHeightD;
			fD.right.leftFootHeight = player.colliders.leftFootTile->GetPosition().x + 16 - tempHeightR;
			fD.up.leftFootHeight = player.colliders.leftFootTile->GetPosition().y + tempHeightU;
			fD.left.leftFootHeight = player.colliders.leftFootTile->GetPosition().x + tempHeightL;

			tempHeightD = player.colliders.rightFootTile->GetHeight(Vector2f(player.colliders.rightFootCol.left, player.colliders.rightFootCol.top), 0);
			tempHeightR = player.colliders.rightFootTile->GetHeight(Vector2f(player.colliders.rightFootCol.left, player.colliders.rightFootCol.top), 3);
			tempHeightU = player.colliders.rightFootTile->GetHeight(Vector2f(player.colliders.rightFootCol.left, player.colliders.rightFootCol.top), 2);
			tempHeightL = player.colliders.rightFootTile->GetHeight(Vector2f(player.colliders.rightFootCol.left, player.colliders.rightFootCol.top), 1);
			fD.down.rightFootHeight = player.colliders.rightFootTile->GetPosition().y + 16 - tempHeightD;
			fD.right.rightFootHeight = player.colliders.rightFootTile->GetPosition().x + 16 - tempHeightR;
			fD.up.rightFootHeight = player.colliders.rightFootTile->GetPosition().y + tempHeightU;
			fD.left.rightFootHeight = player.colliders.rightFootTile->GetPosition().x + tempHeightL;




			hasCollided = true;
			if (player.state.groundMode == GROUNDMODE_FLOOR && fD.down.rightFootHeight <= fD.down.leftFootHeight ||
				player.state.groundMode == GROUNDMODE_RIGHTWALL && fD.right.rightFootHeight <= fD.right.leftFootHeight ||
				player.state.groundMode == GROUNDMODE_CEILING && fD.up.rightFootHeight >= fD.up.leftFootHeight ||
				player.state.groundMode == GROUNDMODE_LEFTWALL && fD.left.rightFootHeight >= fD.left.leftFootHeight)
				player.state.angle = player.colliders.rightFootTile->tile->GetAngle(player.state.groundMode);
			else
				player.state.angle = player.colliders.leftFootTile->tile->GetAngle(player.state.groundMode);

			float downAngDebug = player.colliders.leftFootTile->tile->GetAngle(0);
			//float rightAngDebug = player.colliders.leftFootTile->tile->GetAngle(3);

			if (player.state.groundMode == GROUNDMODE_FLOOR)
				player.state.position = Vector2f(player.state.position.x, std::min(fD.down.leftFootHeight, fD.down.rightFootHeight) - player.state.offsetFromGround);
			else if (player.state.groundMode == GROUNDMODE_RIGHTWALL)
				player.state.position = Vector2f(std::min(fD.right.leftFootHeight, fD.right.rightFootHeight) - player.state.offsetFromGround, player.state.position.y);
			else if (player.state.groundMode == GROUNDMODE_CEILING)
				player.state.position = Vector2f(player.state.position.x, std::max(fD.up.leftFootHeight, fD.up.rightFootHeight) + player.state.offsetFromGround);
			else if (player.state.groundMode == GROUNDMODE_LEFTWALL)
				player.state.position = Vector2f(std::max(fD.left.leftFootHeight, fD.left.rightFootHeight) + player.state.offsetFromGround, player.state.position.y);
		}
		else if (player.colliders.leftFootTile) {
			float tempHeightD = player.colliders.leftFootTile->GetHeight(Vector2f(player.colliders.leftFootCol.left, player.colliders.leftFootCol.top), 0);
			float tempHeightR = player.colliders.leftFootTile->GetHeight(Vector2f(player.colliders.leftFootCol.left, player.colliders.leftFootCol.top), 3);
			float tempHeightU = player.colliders.leftFootTile->GetHeight(Vector2f(player.colliders.leftFootCol.left, player.colliders.leftFootCol.top), 2);
			float tempHeightL = player.colliders.leftFootTile->GetHeight(Vector2f(player.colliders.leftFootCol.left, player.colliders.leftFootCol.top), 1);
			QuadFootSensorData& fD = player.colliders.footData;
			fD.down.leftFootHeight = player.colliders.leftFootTile->GetPosition().y + 16 - tempHeightD;
			fD.right.leftFootHeight = player.colliders.leftFootTile->GetPosition().x + 16 - tempHeightR;
			fD.up.leftFootHeight = player.colliders.leftFootTile->GetPosition().y + tempHeightU;
			fD.left.leftFootHeight = player.colliders.leftFootTile->GetPosition().x + tempHeightL;
			hasCollided = true;

			player.state.angle = player.colliders.leftFootTile->tile->GetAngle(player.state.groundMode);

			if (player.state.groundMode == GROUNDMODE_FLOOR)
				player.state.position = Vector2f(player.state.position.x, fD.down.leftFootHeight - player.state.offsetFromGround);

			else if (player.state.groundMode == GROUNDMODE_RIGHTWALL) {
				player.state.position = Vector2f(fD.right.leftFootHeight - player.state.offsetFromGround, player.state.position.y);
				if (player.colliders.leftFootTile->tile->GetAngle(3) == 270) {
					if (player.sprite.sheetS.getPosition().x > player.colliders.leftFootTile->GetPosition().x)
						player.state.angle = 0;
				}
			}
			else if (player.state.groundMode == GROUNDMODE_CEILING)
				player.state.position = Vector2f(player.state.position.x, fD.up.leftFootHeight + player.state.offsetFromGround);
			else if (player.state.groundMode == GROUNDMODE_LEFTWALL)
				player.state.position = Vector2f(fD.left.leftFootHeight + player.state.offsetFromGround, player.state.position.y);


		}
		else if (player.colliders.rightFootTile) {
			float tempHeightD = player.colliders.rightFootTile->GetHeight(Vector2f(player.colliders.rightFootCol.left, player.colliders.rightFootCol.top), 0);
			float tempHeightR = player.colliders.rightFootTile->GetHeight(Vector2f(player.colliders.rightFootCol.left, player.colliders.rightFootCol.top), 3);
			float tempHeightU = player.colliders.rightFootTile->GetHeight(Vector2f(player.colliders.rightFootCol.left, player.colliders.rightFootCol.top), 2);
			float tempHeightL = player.colliders.rightFootTile->GetHeight(Vector2f(player.colliders.rightFootCol.left, player.colliders.rightFootCol.top), 1);
			QuadFootSensorData& fD = player.colliders.footData;
			fD.down.rightFootHeight = player.colliders.rightFootTile->GetPosition().y + 16 - tempHeightD;
			fD.right.rightFootHeight = player.colliders.rightFootTile->GetPosition().x + 16 - tempHeightR;
			fD.up.rightFootHeight = player.colliders.rightFootTile->GetPosition().y + tempHeightU;
			fD.left.rightFootHeight = player.colliders.rightFootTile->GetPosition().x + tempHeightL;
			hasCollided = true;

			player.state.angle = player.colliders.rightFootTile->tile->GetAngle(player.state.groundMode);
			if (player.state.groundMode == GROUNDMODE_FLOOR)
				player.state.position = Vector2f(player.state.position.x, fD.down.rightFootHeight - player.state.offsetFromGround);
			else if (player.state.groundMode == GROUNDMODE_RIGHTWALL) {
				player.state.position = Vector2f(fD.right.rightFootHeight - player.state.offsetFromGround, player.state.position.y);
				if (player.colliders.rightFootTile->tile->GetAngle(3) == 270) {
					if (player.sprite.sheetS.getPosition().x > player.colliders.rightFootTile->GetPosition().x)
						player.state.angle = 0;
				}

			}
			else if (player.state.groundMode == GROUNDMODE_CEILING)
				player.state.position = Vector2f(player.state.position.x, fD.up.rightFootHeight + player.state.offsetFromGround);
			else if (player.state.groundMode == GROUNDMODE_LEFTWALL)
				player.state.position = Vector2f(fD.left.rightFootHeight + player.state.offsetFromGround, player.state.position.y);
		}
		else {
			player.state.airborne = true;
			player.state.angle = LerpDegrees(player.state.angle, 360, 0.8f);
		}

		for (auto currentTile : GetTilesWithinRange(player.colliders.pushCol, player.state.layer)) {
			if (currentTile->CheckCollision(player.colliders.pushCol, player.state)) {
				if (player.state.groundMode == GROUNDMODE_FLOOR) {
					if (player.colliders.pushCol.top >= currentTile->GetPosition().y &&
						player.colliders.pushCol.top <= currentTile->GetPosition().y + 16) {

						int localPos = player.colliders.pushCol.left + player.colliders.pushCol.width - currentTile->GetPosition().x;
						localPos = bound(0, localPos, 15);

						if (player.colliders.pushCol.left < currentTile->GetPosition().x && player.state.speed.x >= 0) { //Pushing right

							if (currentTile->tile &&
								currentTile->GetPosition().y + 16 - currentTile->tile->GetHeightArray(0)->at(localPos) <= player.colliders.pushCol.top &&
								currentTile->GetPosition().y + currentTile->tile->GetHeightArray(2)->at(localPos) > player.colliders.pushCol.top) {
								player.state.pushState = Direction::Right;
								player.state.speed.x = std::min(player.state.speed.x, 0.0f);
								player.state.groundSpeed = std::min(player.state.groundSpeed, 0.0);
								player.state.position.x = currentTile->GetPosition().x - 10;
								player.colliders.rightPushTile = currentTile;

							}
						}
						else if (player.colliders.pushCol.left > currentTile->GetPosition().x && player.state.speed.x <= 0) { //Pushing left
							if (currentTile->tile &&
								currentTile->GetPosition().y + 16 - currentTile->tile->GetHeightArray(0)->at(localPos) <= player.colliders.pushCol.top &&
								currentTile->GetPosition().y + currentTile->tile->GetHeightArray(2)->at(localPos) > player.colliders.pushCol.top) {
								player.state.pushState = Direction::Left;
								player.state.speed.x = std::max(player.state.speed.x, 0.0f);
								player.state.groundSpeed = std::max(player.state.groundSpeed, 0.0);
								player.state.position.x = currentTile->GetPosition().x + TILE_HEIGHT + 10;
								player.colliders.leftPushTile = currentTile;

							}
						}
					}
				}
			}
		}



		if (hasCollided && player.state.airborne) {

			
			if (player.state.dropdashing) {
				//player.state.groundSpeed = std::max(std::min((std::abs(player.state.groundSpeed)), 16.), 4.) * player.state.lastDirection;
				player.state.groundSpeed = std::abs(player.state.speed.y) * player.state.lastDirection;
				player.state.rolling = true;
				player.state.dropdashing = false;
			} else {
				player.state.rolling = false;
				player.state.groundSpeed = CalculateGSPD(player.state.angle, player.state);
			}
		}

		if (hasCollided) {
			player.state.jumping = false;
			player.state.airborne = false;
		}
		else
			player.state.airborne = true;

		if (player.state.airborne) {
			player.colliders.leftFootTile = nullptr;
			player.colliders.rightFootTile = nullptr;
			//player.state.angle = 0;
		}


	}


	//End of sub-frame updates
	if ((!hasAcc || player.state.rolling) && !player.state.airborne) {
		if (abs(player.state.groundSpeed) > physics.frc) {
			if (player.state.rolling)
				player.state.groundSpeed += ((physics.frc * -GetSign(player.state.groundSpeed)) / 2);
			else if ((!Keyboard::isKeyPressed(Keyboard::D) && !Keyboard::isKeyPressed(Keyboard::A)) || player.state.crouched)
				player.state.groundSpeed += (physics.frc * -GetSign(player.state.groundSpeed));
		}
		else
			player.state.groundSpeed = 0;
	}

	if (player.state.dropdashing) {
		player.state.groundSpeed += 0.2f;
	}
	//Slope physics
	if (!player.state.airborne) {
		player.state.speed.x = player.state.groundSpeed * cos(Rad(player.state.angle));
		player.state.speed.y = player.state.groundSpeed * sin(Rad(player.state.angle));
	}



	player.sprite.animationtimer += (1 / player.sprite.frameduration);

	if (!player.state.rolling) {
		if (player.colliders.leftFootTile && player.colliders.rightFootTile)
			player.sprite.sheetS.setRotation(LerpDegrees((LerpDegrees(player.colliders.leftFootTile->tile->GetAngle(player.state.groundMode), player.colliders.rightFootTile->tile->GetAngle(player.state.groundMode), 0.5f)), player.sprite.sheetS.getRotation(), 0.8f));
		else if (player.colliders.leftFootTile)
			player.sprite.sheetS.setRotation(LerpDegrees(player.colliders.leftFootTile->tile->GetAngle(player.state.groundMode), player.sprite.sheetS.getRotation(), 0.8f));
		else if (player.colliders.rightFootTile)
			player.sprite.sheetS.setRotation(LerpDegrees(player.colliders.rightFootTile->tile->GetAngle(player.state.groundMode), player.sprite.sheetS.getRotation(), 0.8f));
		else
			player.sprite.sheetS.setRotation(LerpDegrees(player.state.angle, player.sprite.sheetS.getRotation(), 0.8f));

		/*if (debugInfoLevel >= DebugInfoLevels::MAX)
		player.sprite.sheetS.setRotation(player.state.angle);*/

	}
	else {
		player.sprite.sheetS.setRotation(0);
	}
	//Camera


	//	gameWindow.setView(View(player.state.position - Vector2f(0, player.sprite.sheetS.getOrigin().y / 2), Vector2f(640, 360))); //Non-dynamic camera

	if (player.state.position.x > camera.position.x) {
		camera.position.x += std::min(player.state.position.x - camera.position.x, 16.f);
	}
	if (player.state.position.y + 20 > camera.position.y + 32) {
		camera.position.y += std::min(player.state.position.y + 20 - camera.position.y - 32, 16.f);
	}
	if (player.state.position.x < camera.position.x - 16) {
		camera.position.x += std::max(player.state.position.x - camera.position.x + 16, -16.f);
	}
	if (player.state.position.y + 20 < camera.position.y - 32) {
		camera.position.y += std::max(player.state.position.y + 20 - camera.position.y + 32, -16.f);
	}


}

void LeaveEditor() {

}

void EventHandling(sf::Event event, GlobalGameState & state, Player & player) {
	if (event.type == Event::Closed)
		state.running = false;
	//UpdateUI(event);


	
	if (event.type == Event::KeyPressed) {
		if (event.key.code == Keyboard::Space) {
			if (!player.state.airborne && !player.state.jumping) {
				if (player.state.crouched) {
					player.state.spindashWindup += 2;
					player.state.spindashing = true;
					//spindash.play();
					if (player.state.spindashWindup > 8)
						player.state.spindashWindup = 8;
				}
				else {
					player.state.horizontalLockTimer = 0;
					player.state.airborne = true;
					player.state.jumping = true;
					player.state.speed.y += player.physics.jmp * cos(Rad(player.state.angle));
					player.state.speed.x -= player.physics.jmp * sin(Rad(player.state.angle));
					//jump.play();
					player.sprite.UpdateRect(player.sprite.xIndex, 2);



					player.state.position += Vector2f(0, player.state.speed.y * (Rad(player.state.angle) / 4)); //Prevent bumping into slope when jumping and walking uphill
				}
			}
			else if (player.state.airborne && player.state.jumping) {
				player.state.groundSpeed = 0;
				player.state.dropdashing = true;
			}
		}
#ifndef _RUNTIMEONLY
		if (event.key.code == Keyboard::B) {
			player.state.groundSpeed = 32 * player.state.lastDirection;
		}



		if (event.key.code == Keyboard::LBracket) {
			state.framerate /= 2.f;
			std::cout << "Game speed: " << state.framerate / 60.f * 100 << "%\n";
		}
		if (event.key.code == Keyboard::RBracket) {
			state.framerate *= 2.f;

			std::cout << "Game speed: " << state.framerate / 60.f * 100 << "%\n";
		}
#endif
	}



}

bool AddToDrawBuffer(const WorldTile & tile, std::unordered_map<std::string, sf::VertexArray> & drawBuffers, sf::RenderTarget & gameWindow, float brightness = 1.0f) {
	if (!tile.tile)
		return false;
	std::string texturename = tile.tile->GetRefInfo().name;
	const sf::VertexArray& drawn = tile.draw(gameWindow, Color(brightness, brightness, brightness, 255));
	if (drawBuffers.count(texturename) == 0) {
		drawBuffers.insert(std::pair<std::string, sf::VertexArray>(texturename, drawn));
		return true;
	}
	for (unsigned int i = 0; i < drawn.getVertexCount(); i++)
		drawBuffers[texturename].append(drawn[i]);
	return true;
}








void UpdateGameUI(sf::RenderTarget& target) {
	using namespace sf;
	static long long int timer;
	static Text txt = Text(currentRoom.name, consolas, 20);
	static Vector2f levelNamePos = { 0.f, (float)(target.getSize().y / 2.f) };
	static float levelNameHVel;
	
	
	if (timer == 0) {
		levelNameHVel = 10.f;
	}

	if (levelNameHVel > 0)
		levelNameHVel-= 0.25f;


	levelNamePos.x += levelNameHVel;

	
	txt.setPosition(0,0);

	if (timer < ULLONG_MAX)
		timer++;

	target.draw(txt);
}





void Draw(RenderWindow& window, Player& player) {
	gameWindow.pushGLStates();
	
	if (player.state.groundMode == GROUNDMODE_FLOOR)
		player.sprite.sheetS.setPosition(player.state.position + Vector2f(0, 1));
	else if (player.state.groundMode == GROUNDMODE_RIGHTWALL)
		player.sprite.sheetS.setPosition(player.state.position + Vector2f(0, 0));
	else if (player.state.groundMode == GROUNDMODE_CEILING)
		player.sprite.sheetS.setPosition(player.state.position + Vector2f(0, -1));
	else
		player.sprite.sheetS.setPosition(player.state.position + Vector2f(-1, 0));
	//player.sprite.sheetS.setOrigin(0.0f, 0.0f);

	//Reset animation frame to 0 if animation changed
	if (player.sprite.prevYIndex != player.sprite.yIndex) {
		player.sprite.prevYIndex = player.sprite.yIndex;
		player.sprite.xIndex = 0;
	}



	player.state.groundMode = std::fmod(round(player.state.angle / 90), 4);
	//if (airborne) groundMode = 0;

	if ((player.state.pushState > 1 && player.state.speed.x < 0) || player.state.pushState < 1 && player.state.speed.x > 0) player.state.pushState = Direction::None; //Small check after scripts update to fix a visual bug

	if (isnan(player.sprite.animationtimer) || isinf(player.sprite.animationtimer))
		player.sprite.animationtimer = 0;

	if (player.sprite.animationtimer < 0)
		player.sprite.animationtimer = 0;
	//Animation
	if (player.sprite.animationtimer >= 1) {

		player.sprite.xIndex++;
		player.sprite.animationtimer = 0;
	}
	if (player.state.jumping || player.state.rolling) {
		if (player.sprite.xIndex > 4)
			player.sprite.xIndex = 0;
		player.sprite.UpdateRect(player.sprite.xIndex, 2);
		player.sprite.frameduration = std::max(5. - std::abs(player.state.groundSpeed), 1.);

	}
	else if (player.state.crouched) {
		if (player.state.spindashing) {
			if (player.sprite.xIndex > 5) 
				player.sprite.xIndex = 1;
			player.sprite.UpdateRect(player.sprite.xIndex, 4);
			player.sprite.frameduration = 1;

		}
		else {
			if (player.sprite.xIndex > 1)
				player.sprite.xIndex = 1;
			player.sprite.UpdateRect(player.sprite.xIndex, 3);
			player.sprite.frameduration = 1;
				
		}
	}
	else {
		if (player.state.groundSpeed != 0 && player.state.pushState == 0) {
			if (std::abs(player.state.groundSpeed) < 5) {
				if (player.sprite.xIndex > 7)
					player.sprite.xIndex = 0;
				player.sprite.UpdateRect(player.sprite.xIndex, 1);
				player.sprite.frameduration = std::max(8 - abs(player.state.groundSpeed), 1.0);
			}
			else {
				if (player.sprite.xIndex > 3)
					player.sprite.xIndex = 0;
				player.sprite.UpdateRect(player.sprite.xIndex, 5);
				player.sprite.frameduration = std::max(8 - abs(player.state.groundSpeed), 1.0);
			}
		}
		else if (player.state.groundSpeed == 0 && (!(Keyboard::isKeyPressed(Keyboard::A) || Keyboard::isKeyPressed(Keyboard::D)) || player.state.airborne)) {
			player.sprite.xIndex = 0;
			player.sprite.UpdateRect(player.sprite.xIndex, 0);
		}
		else if (player.state.pushState != 0) {
			player.sprite.frameduration = 32;
			if (player.sprite.xIndex > 3)
				player.sprite.xIndex = 0;
			player.sprite.UpdateRect(player.sprite.xIndex, 6);
		}
	}
	player.sprite.sheetS.setTexture(player.sprite.sheetT);
	player.sprite.sheetS.setOrigin(player.sprite.origin);
	IntRect rect = player.sprite.GetRect();
	player.sprite.sheetS.setTextureRect(rect);

	debugdrawcalls = 0;


	//sonicLastAngle = player.state.angle;


	camera.position.x = std::max(camera.position.x, gameWindow.getSize().x / 2.f * camera.zoomFactor);
	camera.position.y = std::max(camera.position.y, gameWindow.getSize().y / 2.f * camera.zoomFactor);
	camera.position.x = std::min(camera.position.x, currentRoom.chunks[0].size() * 256 - gameWindow.getSize().x / 2.f * camera.zoomFactor);
	camera.position.y = std::min(camera.position.y, currentRoom.chunks[0][0].size() * 256 - gameWindow.getSize().y / 2.f * camera.zoomFactor);
	//Prevent .5 in camera X and Y, which causes rounding errors in sprite sheets.
	if (camera.position.x - std::floor(camera.position.x) == 0.5)
		camera.position.x -= 0.05f;
	if (camera.position.y - std::floor(camera.position.y) == 0.5)
		camera.position.y -= 0.05f;

	sf::View view(camera.position + camera.offset, Vector2f(gameWindow.getSize()));
	gameWindow.setView(view);


	using namespace sf;
	Vector2i upLeftScrTile = (Vector2i(camera.position) - (Vector2i(Vector2f(gameWindow.getSize()) * camera.zoomFactor) / 2)) / 16; //Upper left tile on screen
	Vector2i downRightScrTile = (Vector2i(camera.position) + (Vector2i(Vector2f(gameWindow.getSize()) * camera.zoomFactor) / 2)) / 16 + Vector2i(1, 1); //Lower right tile on screen
	upLeftScrTile.x = std::max((int)upLeftScrTile.x, 0);
	upLeftScrTile.y = std::max((int)upLeftScrTile.y, 0);
	downRightScrTile.x = std::min((int)downRightScrTile.x, 16 * 64);
	downRightScrTile.y = std::min((int)downRightScrTile.y, 16 * 8);
	//gameWindow.clear(Color::Black);
	player.sprite.fillColor.r = 255;
	player.sprite.fillColor.g = 255;
	player.sprite.fillColor.b = 255;
	player.sprite.fillColor.a = 255;
	player.sprite.sheetS.setColor(player.sprite.fillColor);



	if (currentRoom.parallaxLayers.size() > 0) {
		for (unsigned int i = 0; i < currentRoom.parallaxLayers.size(); i++) {
			ParallaxLayer& currentPL = currentRoom.parallaxLayers[i];

			currentPL.position = currentPL.origin + (camera.position - Vector2f(gameWindow.getSize()) / 2.f) * Vector2f(currentPL.scrollMultiplier.x, currentPL.scrollMultiplier.y);
				
			currentPL.origin += currentPL.constScrollRate;
			currentPL.origin.x = std::fmod(currentPL.origin.x, (float)currentPL.image->getSize().x);
			//currentPL.origin.y = std::fmod(currentPL.origin.y, (float)currentPL.image->getSize().y);

			sf::Vector2i currentRequiredIndex(
				(
				(camera.position - sf::Vector2f(gameWindowSize / 2))
					- currentPL.position
					)
				/ sf::Vector2f(currentPL.image->getSize()));


			sf::VertexArray quad(sf::Quads, 4);
			if ((currentPL.hLoop || currentPL.yLoop)) {

				//int maxDrawIndex = currentRequiredIndex.x
				//	+ camera.position.x / currentPL.image->getSize().x * std::ceilf((float)gameWindowSize.x / (float)currentPL.image->getSize().x);
					//+ std::ceilf((float)gameWindowSize.x / (float)currentPL.image->getSize().x);
				int maxDrawIndex = currentRequiredIndex.x;
				maxDrawIndex += std::ceilf((float)gameWindowSize.x / (float)currentPL.image->getSize().x) + 1;
				for (int i = currentRequiredIndex.x; i < maxDrawIndex; i++) {
					sf::Vector2f position(currentPL.position.x + (currentPL.image->getSize().x * i), currentPL.position.y);
					quad[0].position = position;
					quad[1].position = position + sf::Vector2f(currentPL.image->getSize().x, 0);
					quad[2].position = position + sf::Vector2f(currentPL.image->getSize().x, currentPL.image->getSize().y);
					quad[3].position = position + sf::Vector2f(0, currentPL.image->getSize().y);
					quad[0].texCoords = { 0, 0 };
					quad[1].texCoords = { (float)currentPL.image->getSize().x, 0 };
					quad[2].texCoords = { (float)currentPL.image->getSize().x, (float)currentPL.image->getSize().y };
					quad[3].texCoords = { 0, (float)currentPL.image->getSize().y };
					gameWindow.draw(quad, currentPL.image);
				}
			}
			else {
					
				quad[0].position = currentPL.position;
				quad[1].position = currentPL.position + sf::Vector2f(currentPL.image->getSize().x, 0);
				quad[2].position = currentPL.position + sf::Vector2f(currentPL.image->getSize().x, currentPL.image->getSize().y);
				quad[3].position = currentPL.position + sf::Vector2f(0, currentPL.image->getSize().y);
					
					
				quad[0].texCoords = { 0, 0 };
				quad[1].texCoords = { (float)currentPL.image->getSize().x, 0 };
				quad[2].texCoords = { (float)currentPL.image->getSize().x, (float)currentPL.image->getSize().y };
				quad[3].texCoords = { 0, (float)currentPL.image->getSize().y };
			}
			gameWindow.draw(quad, currentPL.image);
		}
	}



	view = View(camera.position + camera.offset, Vector2f(gameWindow.getSize()) * camera.zoomFactor);
	gameWindow.setView(view);


	RectangleShape drawrect(Vector2f(17, 17));
	drawrect.setOutlineColor(Color(255, 255, 255, 25));
	drawrect.setFillColor(Color::Transparent);
	drawrect.setOutlineThickness(-1 * std::max(camera.zoomFactor, 1.f));


	sf::Sprite refspr(referenceimage);
	refspr.setColor(Color(255, 255, 255, 125));
	gameWindow.draw(refspr);

	std::unordered_map<std::string, sf::VertexArray> drawBuffers;

	for (int l = 0; l < player.state.layer; l++) { //Layer
		for (int x = upLeftScrTile.x; x < downRightScrTile.x; x++) {
			for (int y = upLeftScrTile.y; y < downRightScrTile.y; y++) {
				Vector2i currentChunk = { x / 16, y / 16 };
				if (emptychunks[x][y] == CHUNK_EMPTY) {
					std::cout << "Skipped chunk.";
					y += 16;
				}

				WorldTile* currentTile = GetTileAtGlobalIndex(Vector2i(x, y), l, &currentRoom);
					
				bool nonempty = AddToDrawBuffer(*currentTile, drawBuffers, gameWindow);
				if (emptychunks[x][y] == CHUNK_UNKNOWN && nonempty)
					emptychunks[x][y] = CHUNK_NONEMPTY;
				if (debugInfoLevel >= DebugInfoLevels::MAX)
					debugdrawcalls++;
			}
		}
	}


	for (auto& b : drawBuffers) {
		gameWindow.draw(b.second, &currentRoom.textures[b.first]);
	}
	for (unsigned int l = 0; l < currentRoom.objects.size(); l++) {
		for (unsigned int o = 0; o < currentRoom.objects[l].size(); o++) {
			HCObject* currentObject = &currentRoom.objects[l][o];

			for (auto& c : *currentObject->GetComponentsPtr()) {
				gameWindow.setActive();
				gameWindow.draw(c);
				gameWindow.setActive(false);
			}
		}
	}
	if (debugInfoLevel >= DebugInfoLevels::MIN) {
		//gameWindow.draw(chunkOutline);
	}

	gameWindow.draw(player.sprite.sheetS);
	Vector2i currentChunkPos(camera.position / Vector2f(chunkSizePx, chunkSizePx));


	for (int l = player.state.layer; l < currentRoom.chunks.size(); l++) { //Layer
		for (int x = upLeftScrTile.x; x < downRightScrTile.x; x++) {
			for (int y = upLeftScrTile.y; y < downRightScrTile.y; y++) {
				if (emptychunks[x][y] == CHUNK_EMPTY)
					y += 16;

				WorldTile * currentTile = GetTileAtGlobalIndex(Vector2i(x, y), l, &currentRoom);

				bool nonempty = AddToDrawBuffer(*currentTile, drawBuffers, gameWindow);
				if (emptychunks[x][y] == CHUNK_UNKNOWN && nonempty)
					emptychunks[x][y] = CHUNK_NONEMPTY;
				if (debugInfoLevel >= DebugInfoLevels::MAX)
					debugdrawcalls++;
			}
		}
	}
	for (auto& b : drawBuffers) {
		gameWindow.draw(b.second, &currentRoom.textures[b.first]);
	}



	for (unsigned int l = player.state.layer; l < currentRoom.objects.size(); l++) {
		for (unsigned int o = 0; o < currentRoom.objects[l].size(); o++) {
			HCObject* currentObject = &currentRoom.objects[l][o];
				
			for (auto& c : *currentObject->GetComponentsPtr()) {
				gameWindow.setActive();
				gameWindow.draw(c);
				gameWindow.setActive(false);
			}
		}
	}


	UpdateGameUI(gameWindow);

#ifndef _RUNTIMEONLY
	if (debugInfoLevel >= DebugInfoLevels::MIN) {
		RectangleShape feetColLDraw = Tile::floatRectToRect(player.colliders.leftFootCol);
		feetColLDraw.setFillColor(Color::Cyan);
		gameWindow.draw(feetColLDraw);

		RectangleShape feetColRDraw = Tile::floatRectToRect(player.colliders.rightFootCol);
		feetColRDraw.setFillColor(Color::Green);
		gameWindow.draw(feetColRDraw);

		feetColLDraw = Tile::floatRectToRect(player.colliders.leftCeilCol);
		feetColLDraw.setFillColor(Color::Cyan);
		gameWindow.draw(feetColLDraw);

		feetColRDraw = Tile::floatRectToRect(player.colliders.rightCeilCol);
		feetColRDraw.setFillColor(Color::Green);
		gameWindow.draw(feetColRDraw);

		gameWindow.draw(Tile::floatRectToRect(player.colliders.pushCol));

		RectangleShape sonicPosDraw = RectangleShape(Vector2f(1, 1));
		sonicPosDraw.setPosition(player.state.position);
		sonicPosDraw.setFillColor(Color::Red);
		gameWindow.draw(sonicPosDraw);

		RectangleShape sonicColRect;
		sonicColRect.setSize(Vector2f(player.sprite.width, player.sprite.height));
		sonicColRect.setOrigin(player.sprite.origin);
		sonicColRect.setRotation(player.sprite.sheetS.getRotation());
		sonicColRect.setPosition(player.sprite.sheetS.getPosition());
		sonicColRect.setFillColor(Color(0, 0, 0, 0));
		sonicColRect.setOutlineColor(Color(0, 0, 255, 125));
		sonicColRect.setOutlineThickness(-1);
		gameWindow.draw(sonicColRect);
		RectangleShape leftFootColPt;
		leftFootColPt.setSize(Vector2f(1, 1));
		leftFootColPt.setPosition(player.colliders.leftFootCol.left, player.colliders.footData.down.leftFootHeight);
		leftFootColPt.setFillColor(Color::Blue);
		gameWindow.draw(leftFootColPt);
	}
	


#endif

	gameWindow.display();

	window.clear(Color(23, 24, 25, 255));
	Texture gameWindText = gameWindow.getTexture();
	gameDraw.setTexture(gameWindText);

	gameDraw.setScale((window.getSize().x / gameWindText.getSize().x), (window.getSize().y / gameWindText.getSize().y));


	gameDraw.setPosition(0, 0);

	window.draw(gameDraw);




#ifndef _RUNTIMEONLY
	if (debugInfoLevel >= DebugInfoLevels::MIN) {
		Text debugInfo;
		debugInfo.setFillColor(Color::White);
		debugInfo.setFont(consolas);
		debugInfo.setCharacterSize(15);
		std::string footNameL = AddressStr((void const*)player.colliders.leftFootTile);

		std::string footNameR = AddressStr((void const*)player.colliders.rightFootTile);
		std::string pushNameL = AddressStr((void const*)player.colliders.leftPushTile);

		std::string pushNameR = AddressStr((void const*)player.colliders.rightPushTile);

		using namespace std;
		debugInfo.setString(
			"HedgehogCreator Alpha - By UltimateNanite\n(Hi Dylan!)\nFOOT: L:0x" + footNameL + ", " + "R: 0x" + footNameR + "\n"
			+ "PUSH: L:0x" + pushNameL + ", " + "R: 0x" + pushNameR + "\n"
			+ "FPS: " + to_string(ceilf(fps)) + "\n"
			+ "POS: " + to_string(player.state.position.x) + ", " + to_string(player.state.position.y) + "\n"
			+ "SPD: " + to_string(player.state.speed.x) + ", " + to_string(player.state.speed.y) + "\n"
			+ "ACC: " + to_string(player.state.speed.x - lastXspd) + ", " + to_string(player.state.speed.y - lastYspd) + "\n"
			+ "GSPD: " + to_string(player.state.groundSpeed) + "\n"
			+ "AIR: " + to_string(player.state.airborne) + "\n"
			+ "SpindashWindup: " + to_string(player.state.spindashWindup) + "\n"
			+ "ANGLE: " + to_string(360 - player.state.angle) + "\n(GM: " + to_string(player.state.groundMode) + ", ROT:" + to_string(player.sprite.sheetS.getRotation()) + ")\n"
			+ "HorLockTim: " + to_string(player.state.horizontalLockTimer) + "\n"
			+ "Pushing: " + to_string(player.state.pushState) + "\n"
			+ "ANIM: " + to_string(player.sprite.xIndex) + ", " + to_string(player.sprite.yIndex) + "\n"
			+ "ANIMTIMER: " + to_string(player.sprite.animationtimer) + "\n"
		);
		debugInfo.setPosition(gameDraw.getPosition());
		window.draw(debugInfo);

	}
#endif
	lastXspd = player.state.speed.x;
	lastYspd = player.state.speed.y;







	window.setView(View(Vector2f(window.getSize().x / 2, window.getSize().y / 2), Vector2f(window.getSize())));
	//Final drawing

	gameWindow.popGLStates();
	window.display();



}


HCObject* GetObjectFromID(int id) {
	return currentRoom.associations[id].parent;
}

