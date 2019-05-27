// HedgehogEditor.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <SFML/Main.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include "Room.h"
#include "imgui.h"
#include "imgui-SFML.h"
#include <stdexcept>
#include <filesystem>
#include "AssetSelector.h"
#include "imgui-extra.h"
#include "TinyFileDialog.h"
Room currentRoom(nullptr);


#define CHUNK_UNKNOWN 0
#define CHUNK_EMPTY 1
#define CHUNK_NONEMPTY 2
short emptychunks[64][8];


std::map<std::string, AssetSelector> assetselectors;


namespace fs = std::filesystem;
Camera cam;

HCObject* selectedObject;



int debugDrawCalls = 0;
int debugDrawnVertices = 0;

sf::RenderTexture gameWindow;
sf::RenderWindow startupDialog;
void update(sf::Time deltaTime, sf::RenderWindow& window);
void render(sf::RenderWindow& window, sf::Time deltaTime, int framesPerSecond);

std::unordered_map<std::string, sf::VertexArray> drawBuffers;
bool AddToDrawBuffer(const WorldTile& tile, std::unordered_map<std::string, sf::VertexArray>& drawBuffers, sf::RenderTarget& gameWindow, float brightness = 1.0f) {
	if (!tile.tile)
		return false;
	std::string texturename = tile.tile->GetRefInfo().name;
	const sf::VertexArray& drawn = tile.draw(gameWindow, sf::Color(brightness, brightness, brightness, 255));
	if (drawBuffers.count(texturename) == 0) {
		drawBuffers.insert(std::pair<std::string, sf::VertexArray>(texturename, drawn));
		return true;
	}
	for (unsigned int i = 0; i < drawn.getVertexCount(); i++)
		drawBuffers[texturename].append(drawn[i]);
	return true;
}
WorldTile* GetTileAtGlobalIndex(sf::Vector2i index, int layer, Room * currentRoom) {
	using namespace sf;
	index.x = std::max(index.x, 0);
	index.y = std::max(index.y, 0);
	Vector2i currentChunk(index.x / 16, index.y / 16);
	Vector2i currentPosInChunk(std::fmod(index.x, 16), std::fmod(index.y, 16));
	return &currentRoom->chunks[layer][currentChunk.x][currentChunk.y][currentPosInChunk.x][currentPosInChunk.y];
}



class UIElement : public sf::Drawable {
protected:
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const = 0;
	sf::Vector2f position;
	sf::Vector2f size;
public:
	sf::Vector2f getPosition() const {
		return position;
	}
	sf::Vector2f getSize() const {
		return size;
	}
	template<typename T>
	void setPosition(sf::Vector2<T> position) {
		this->position = sf::Vector2f(position);
	}
	template<typename T>
	void setSize(sf::Vector2<T> size) {
		this->size = sf::Vector2f(size);
	}

	bool checkCollision(sf::Vector2f pos) const {
		return (
			pos.x > position.x &&
			pos.y > position.y &&
			pos.x < position.x + size.x &&
			pos.y < position.y + size.y
			);
	}
	virtual void focusUpdate() = 0;
	virtual void endFocusUpdate() = 0;
	virtual void nonFocusUpdate() = 0;
	virtual void regularUpdate() = 0;

};

class Button : public UIElement {
private:
	sf::Text text;
	std::function<void()> callOnClick;
public:
	Button(sf::FloatRect rect, std::function<void()> onCall = std::function<void()>(), sf::Text text = sf::Text())
		: callOnClick(onCall), text(text) {
		position = { rect.left, rect.top };
		size = { rect.width, rect.height };
	}
	virtual void focusUpdate() {
		callOnClick();
	}
	virtual void endFocusUpdate() {

	}
	virtual void nonFocusUpdate() {

	}
	virtual void regularUpdate() {

	}
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const {
		using namespace sf;
		RectangleShape button;
		button.setPosition(position);
		button.setSize(size);
		if (
			checkCollision(Vector2f(Mouse::getPosition(startupDialog))) &&
			!Mouse::isButtonPressed(Mouse::Left)
			) {
			button.setFillColor(Color(81, 77, 117));
		}
		else {
			button.setFillColor(Color(65, 62, 94));
		}
		Text drawTxt = text;
		drawTxt.setPosition(
			position +
			(size / 2.f) -
			Vector2f(drawTxt.getGlobalBounds().width / 2, drawTxt.getCharacterSize() / 2)
		);

		target.draw(button);
		target.draw(drawTxt);
	}
};


sf::Vector2i tileDrawIndex;
std::string currentTexture = "CPZ.png";


bool runningOpenDialog = true;
bool running = true;
UIElement * currentFocus;

void openRoom() {
	std::string filepath_name = tinyfd_openFileDialog("Open room...", GetOwnFilePath().c_str(), 0, nullptr, nullptr, 0);
	fs::path dir(filepath_name);
	if (!(fs::exists(dir)))
		throw std::runtime_error("File \"" + filepath_name + "\" not found.");
	
	std::ifstream ifs(filepath_name);
	
	std::string filepath = filepath_name;
	if (filepath_name.find_last_of('/') != -1)
		filepath = filepath_name.substr(0, filepath_name.find_last_of('/') + 1);
	if (filepath_name.find_last_of('\\') != -1)
		filepath = filepath_name.substr(0, filepath_name.find_last_of('\\') + 1);
	currentRoom.path = filepath;
	
	std::string filename = filepath_name;
	if (filepath_name.find_last_of('/') != -1)
		filename = filepath_name.substr(filepath_name.find_last_of('/') + 1);
	if (filepath_name.find_last_of('\\') != -1)
		filename = filepath_name.substr(filepath_name.find_last_of('\\') + 1);
	currentRoom.filename = filename;
	ifs >> currentRoom;
	runningOpenDialog = false;
	ifs.close();
}
void newRoom() {
	currentRoom = Room(nullptr);
	runningOpenDialog = false;
}

void saveRoom() {

	std::cout << "Saving...\n";
	std::string path;
	if (currentRoom.path == "") {
		path = tinyfd_saveFileDialog("Save room..", GetOwnFilePath().c_str(), 0, 0, "");
	}
	else {
		path = currentRoom.path + currentRoom.filename;
	}
	fs::path dir(path);
	if (!(fs::exists(dir))) {
		throw std::exception("File not found.");
	}

	std::ofstream ofs(path);

	ofs << currentRoom;

	ofs.close();

}
void exitDialog() {
	runningOpenDialog = false;
	running = false;
}

sf::Text newStringToText(const sf::Text text, std::string str) {
	using namespace sf;
	Text result = text;
	result.setString(str);
	return result;
}
sf::Font OpenSans;

class Tools {
public:
	static const int Select = 0;
	static const int Pencil = 1;
	static const int Brush = 2;
	static const int FloodFill = 3;
	static const int Eraser = 4;
};

int tool = Tools::Select;

std::vector<WorldTile*> selectedTiles;	
int currentLayer = 0;


sf::Vector2f hcMouseToWorld(sf::Vector2i mouse, const Camera& camera, sf::Vector2u drawWindowSize) {
	//Magic black box that SOMEHOW transforms the mousePos into world coordinates.
	sf::Vector2f mousePos = sf::Vector2f(mouse);
	mousePos /= 2.f;
	mousePos *= camera.zoomFactor;
	mousePos += camera.position - (sf::Vector2f(drawWindowSize) * (0.5f * camera.zoomFactor));
	return mousePos;
}
float previewOpacity = 0.f;
bool useTool(sf::Event::MouseButtonEvent eventparams, const sf::RenderTexture& drawWindow, const sf::RenderWindow& window, const Camera& camera) {
	
	
	//Magic black box that SOMEHOW transforms the mousePos into world coordinates.
	//sf::Vector2f mousePos = sf::Vector2f(sf::Mouse::getPosition(window));
	//mousePos /= 2.f;
	//mousePos *= camera.zoomFactor;
	//mousePos += camera.position - (sf::Vector2f(drawWindow.getSize()) * (0.5f * camera.zoomFactor));
	sf::Vector2f mousePos = hcMouseToWorld(sf::Mouse::getPosition(window), camera, drawWindow.getSize());
	


	sf::Vector2i currentChunkPos(sf::Vector2i(mousePos) / 256);
	sf::Vector2i inChunkPos(std::fmod(mousePos.x, 256) / 16, std::fmod(mousePos.y, 256) / 16);

	if (currentChunkPos.x < 0 || currentChunkPos.y < 0 || currentChunkPos.x >= 64 || currentChunkPos.y >= 8 ||
		inChunkPos.x < 0 || inChunkPos.y < 0 || inChunkPos.x >= 16 || inChunkPos.y >= 16) {
		std::cout << "Can't place tile at (" << (currentChunkPos.x + inChunkPos.x) << ", " << (currentChunkPos.y + inChunkPos.y) << ")\n";
		return false;
	}

	WorldTile& currentTile = currentRoom.chunks[currentLayer][currentChunkPos.x][currentChunkPos.y][inChunkPos.x][inChunkPos.y];

	
		

	WorldTile placetile = WorldTile();
	WorldTile emptytile = WorldTile();


	placetile.inChunkPos = sf::Vector2<unsigned short>(inChunkPos);
	placetile.chunk = sf::Vector2<unsigned short>(currentChunkPos);

	emptytile.inChunkPos = sf::Vector2<unsigned short>(inChunkPos);
	emptytile.chunk = sf::Vector2<unsigned short>(currentChunkPos);
	emptytile.tile = nullptr;
	if (currentRoom.tilemapdata.count(currentTexture) > 0) {
		placetile.tile = &currentRoom.tilemapdata[currentTexture][tileDrawIndex.x][tileDrawIndex.y];
	}
	else {
		placetile.tile = nullptr;
	}
	
	switch (tool) {
		 
	case (Tools::Pencil): {
		if (eventparams.button == sf::Mouse::Left)
			currentTile = placetile;
		else {
			currentTile = emptytile;
		}

		break;
	}
	case (Tools::Select): {
		
		if (eventparams.button == sf::Mouse::Left) {
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) {
				if (&currentTile && std::find(selectedTiles.begin(), selectedTiles.end(), &currentTile) == selectedTiles.end())
					selectedTiles.push_back(&currentTile);
			}
			else {
				selectedTiles.clear();

				if (&currentTile)
					selectedTiles.push_back(&currentTile);
			}
		}

	}
	case (Tools::Brush): {


		break;
	}

	case (Tools::Eraser): {


		break;
	}

	case (Tools::FloodFill): {


		break;

	}

	}
	return true;
}





int main()
{
	using namespace sf;


	startupDialog.create(VideoMode(700, 400), "HedgehogEditor", Style::None);

	Text UITXT;

	OpenSans.loadFromFile("Fonts/OpenSans-Regular.ttf");

	UITXT.setFont(OpenSans);
	UITXT.setFillColor(Color::White);
	UITXT.setCharacterSize(20);

	std::vector<UIElement*> uielements = {
		new Button(FloatRect(100, 100, 200, 100), newRoom, newStringToText(UITXT, "New Room")),
		new Button(FloatRect(400, 100, 200, 100), openRoom, newStringToText(UITXT, "Open Room")),
		new Button(FloatRect(700 - 25, 0, 25, 25), exitDialog, newStringToText(UITXT, "X")),
	};

	while (runningOpenDialog) {

		Event event;
		while (startupDialog.pollEvent(event)) {

			if (event.type == Event::Closed) {
				runningOpenDialog = false;
				running = false;
			}

			if (event.type == Event::MouseButtonPressed) {
				UIElement* endFocus = currentFocus;
				currentFocus = nullptr;
				for (auto uie : uielements) {
					if (uie->checkCollision(sf::Vector2f(event.mouseButton.x, event.mouseButton.y))) {
						currentFocus = uie;
					}
				}
				if (endFocus && endFocus != currentFocus)
					endFocus->endFocusUpdate();
			}

			
		}
		for (auto uie : uielements) {
			uie->regularUpdate();
			if (uie != currentFocus)
				uie->nonFocusUpdate();
		}
		if (currentFocus) {
			currentFocus->focusUpdate();
		}

		startupDialog.clear(Color(29, 23, 51));
		for (auto uie : uielements) {
			startupDialog.draw(*uie);
		}

		startupDialog.display();

	}

	for (std::vector< UIElement* >::iterator it = uielements.begin(); it != uielements.end(); ++it)
	{
		delete (*it);
	}

	uielements.clear();
	startupDialog.close();

	if (!running) return 0;
	RenderWindow window(VideoMode(1280, 720), "HedgehogEditor");


	gameWindow.create(window.getSize().x / 2, window.getSize().y / 2);

	ImGui::SFML::Init(window);
	ImGui::NewFrame();
#pragma region Dear ImGUI style
	ImGuiStyle * style = &ImGui::GetStyle();

	style->WindowPadding = ImVec2(15, 15);
	style->WindowRounding = 5.0f;
	style->FramePadding = ImVec2(5, 5);
	style->FrameRounding = 4.0f;
	style->ItemSpacing = ImVec2(12, 8);
	style->ItemInnerSpacing = ImVec2(8, 6);
	style->IndentSpacing = 25.0f;
	style->ScrollbarSize = 15.0f;
	style->ScrollbarRounding = 9.0f;
	style->GrabMinSize = 5.0f;
	style->GrabRounding = 3.0f;

	style->Colors[ImGuiCol_Text] = ImVec4(0.80f, 0.80f, 0.83f, 1.00f);
	style->Colors[ImGuiCol_TextDisabled] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
	style->Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style->Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
	style->Colors[ImGuiCol_PopupBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
	style->Colors[ImGuiCol_Border] = ImVec4(0.80f, 0.80f, 0.83f, 0.88f);
	style->Colors[ImGuiCol_BorderShadow] = ImVec4(0.92f, 0.91f, 0.88f, 0.00f);
	style->Colors[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
	style->Colors[ImGuiCol_FrameBgActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 0.98f, 0.95f, 0.75f);
	style->Colors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
	style->Colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
	style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	//style->Colors[ImGuiCol_ComboBg] = ImVec4(0.19f, 0.18f, 0.21f, 1.00f);
	style->Colors[ImGuiCol_CheckMark] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
	style->Colors[ImGuiCol_SliderGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
	style->Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style->Colors[ImGuiCol_Button] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
	style->Colors[ImGuiCol_ButtonActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_Header] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_HeaderHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_HeaderActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style->Colors[ImGuiCol_Column] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_ColumnHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
	style->Colors[ImGuiCol_ColumnActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	style->Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	//style->Colors[ImGuiCol_CloseButton] = ImVec4(0.40f, 0.39f, 0.38f, 0.16f);
	//style->Colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.40f, 0.39f, 0.38f, 0.39f);
	//style->Colors[ImGuiCol_CloseButtonActive] = ImVec4(0.40f, 0.39f, 0.38f, 1.00f);
	style->Colors[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
	style->Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
	style->Colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
	style->Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
	style->Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25f, 1.00f, 0.00f, 0.43f);
	style->Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(1.00f, 0.98f, 0.95f, 0.73f);
	ImGuiIO& io = ImGui::GetIO();

	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
																//io.Fonts->Clear();
	//io.Fonts->AddFontDefault();
	//io.Fonts->AddFontFromFileTTF((GetOwnFilePath() + "/Default/Fonts/Ruda-Bold.ttf").c_str(), 12);
	//io.Fonts->AddFontFromFileTTF((GetOwnFilePath() + "/Default/Fonts/Ruda-Bold.ttf").c_str(), 10);
	//io.Fonts->AddFontFromFileTTF((GetOwnFilePath() + "/Default/Fonts/Ruda-Bold.ttf").c_str(), 14);
	//io.Fonts->AddFontFromFileTTF((GetOwnFilePath() + "/Default/Fonts/Ruda-Bold.ttf").c_str(), 18);
	//ImGui::SFML::UpdateFontTexture();
#pragma endregion
	ImGui::EndFrame();


	Clock deltaClock;
	Time deltaTime;
	Time averageDeltaTime;
	Clock averageDeltaClock;
	int framesForAverage = 0;
	Clock avgMsClock;
	while (running) {
		

		try {
			Event event;
			while (window.pollEvent(event)) {

				ImGui::SFML::ProcessEvent(event);
				if (event.type == Event::Closed) {
					running = false;
					break;
				}
				if (event.type == Event::Resized) {
					window.setView(View(Vector2f(event.size.width / 2, event.size.height / 2), Vector2f(event.size.width, event.size.height)));
					gameWindow.create(window.getSize().x / 2, window.getSize().y / 2);
				}
				if (!ImGui::GetIO().WantCaptureMouse) {
					//Mouse input

					if (event.type == Event::MouseButtonPressed) {
						useTool(event.mouseButton, gameWindow, window, cam);
					}
					if (event.type == Event::MouseWheelScrolled) {
						std::cout << event.mouseWheelScroll.y << std::endl;
						if (event.mouseWheelScroll.delta > 0)
							cam.zoomFactor /= 2;
						else if (event.mouseWheelScroll.delta < 0)
							cam.zoomFactor *= 2;
					}
				}
				if (!ImGui::GetIO().WantCaptureKeyboard) {
					//Keyboard input
					if (event.type == Event::KeyPressed) {
						switch (event.key.code) {
						
						case Keyboard::Left:
							previewOpacity = 100.f;
							tileDrawIndex.x--;
							break;
						case Keyboard::Right:
							previewOpacity = 100.f;
							tileDrawIndex.x++;
							break;
						case Keyboard::Up:
							previewOpacity = 100.f;
							tileDrawIndex.y--;
							break;
						case Keyboard::Down:
							previewOpacity = 100.f;
							tileDrawIndex.y++;
							break;
						case Keyboard::Delete:
							for (auto selectedTile : selectedTiles) {
								selectedTile->tile = nullptr;
							}
							selectedTiles.clear();
							break;
						}
					}
					sf::Vector2i maxIndex = sf::Vector2i(currentRoom.textures[currentTexture].getSize()) / 16;
					if (tileDrawIndex.x < 0) tileDrawIndex.x = 0;
					if (tileDrawIndex.y < 0) tileDrawIndex.y = 0;
					if (tileDrawIndex.x >= maxIndex.x) tileDrawIndex.x = maxIndex.x - 1;
					if (tileDrawIndex.y >= maxIndex.y) tileDrawIndex.y = maxIndex.y - 1;

				}
			}
			deltaTime = deltaClock.restart();
			if (!window.hasFocus()) continue;

			update(deltaTime, window);


			ImGui::SFML::Update(window, deltaTime);

			render(window, averageDeltaTime, framesForAverage);
			
			ImGui::SFML::Render(window);
			
			framesForAverage++;
			if (averageDeltaClock.getElapsedTime().asSeconds() >= 1.f) {
				averageDeltaTime = averageDeltaClock.restart() / (float)framesForAverage;
				framesForAverage = 0;
			}
			window.display();
		}
		catch (std::runtime_error &e) {
			std::cout << "An unhandled runtime error occured: " << e.what() << std::endl;
		}
		catch (std::exception &e) {
			std::cout << "An unhandled exception occured: " << e.what() << std::endl;
		}
	}
	ImGui::SFML::Shutdown();
}




sf::Vector2f getWorldMousePos(sf::RenderWindow& window) {
	using namespace sf;
	return hcMouseToWorld(Mouse::getPosition(window), cam, gameWindow.getSize());
}


sf::Vector2f prevMousePos;



void update(sf::Time deltaTime, sf::RenderWindow& window) {
	using namespace sf;
	double deltaFactor = deltaTime.asSeconds() / (1.0 / 60.0);


	if (Keyboard::isKeyPressed(Keyboard::A)) {
		cam.position.x -= 10 * deltaFactor;
	}
	if (Keyboard::isKeyPressed(Keyboard::D)) {
		cam.position.x += 10 * deltaFactor;
	}
	if (Keyboard::isKeyPressed(Keyboard::W)) {
		cam.position.y -= 10 * deltaFactor;
	}
	if (Keyboard::isKeyPressed(Keyboard::S)) {
		cam.position.y += 10 * deltaFactor;
	}

	for (auto& l : currentRoom.objects) {
		for (auto& currentObject : l) {
			for (auto& component : *currentObject.GetComponentsPtr()) {
				component.Update(deltaTime);
			}
		}
	}
	if (!ImGui::GetIO().WantCaptureMouse) {
		if (sf::Mouse::isButtonPressed(Mouse::Left) && tool == Tools::Select) {
			selectedObject = nullptr;
			for (auto& l : currentRoom.objects) {
				for (auto& obj : l) {
					if (sf::FloatRect(obj.GetPosition(), obj.GetSize()).contains(getWorldMousePos(window)))
						selectedObject = &obj;
				}
			}
		}

		if (Mouse::isButtonPressed(Mouse::Left) && selectedObject) {
			Vector2f mouseMov = getWorldMousePos(window) - prevMousePos;
			selectedObject->position += mouseMov;
		}

		prevMousePos = getWorldMousePos(window);
	}

}


void drawGrid(sf::RenderTarget& target, sf::Vector2i upLeftScrTile, sf::Vector2i downRightScrTile) {
	using namespace sf;
	VertexArray horGrid(sf::Quads);
	Vertex upLeft;
	Vertex upRight;
	Vertex downRight;
	Vertex downLeft;
	uint8_t gridOpacity = 25;
	upLeft.color = Color(225, 255, 255, gridOpacity);
	upRight.color = Color(255, 255, 255, gridOpacity);
	downRight.color = Color(255, 255, 255, gridOpacity);
	downLeft.color = Color(255, 255, 255, gridOpacity);
	for (int y = upLeftScrTile.y; y < downRightScrTile.y; y++) {
		upLeft.position =		{ 0,                         y * 16.f };
		upRight.position =		{ downRightScrTile.x * 16.f, y * 16.f };
		downRight.position =	{ downRightScrTile.x * 16.f, y * 16.f + cam.zoomFactor };
		downLeft.position =		{ 0,                         y * 16.f + cam.zoomFactor };
		horGrid.append(upLeft);
		horGrid.append(upRight);
		horGrid.append(downRight);
		horGrid.append(downLeft);
	}
	for (int x = upLeftScrTile.x; x < downRightScrTile.x; x++) {
		upLeft.position =		{ x * 16.f,                  0 };
		upRight.position =		{ x * 16.f + cam.zoomFactor, 0 };
		downRight.position =	{ x * 16.f + cam.zoomFactor, downRightScrTile.y * 16.f };
		downLeft.position =		{ x * 16.f,					 downRightScrTile.y * 16.f };
		horGrid.append(upLeft);
		horGrid.append(upRight);
		horGrid.append(downRight);
		horGrid.append(downLeft);
	}

	//Chunks
	upLeft.color = Color(66, 164, 244, gridOpacity*gridOpacity);
	upRight.color = Color(66, 164, 244, gridOpacity * gridOpacity);
	downRight.color = Color(66, 164, 244, gridOpacity * gridOpacity);
	downLeft.color = Color(66, 164, 244, gridOpacity * gridOpacity);
	
	for (int y = upLeftScrTile.y; y < downRightScrTile.y; y++) {
		if (std::fmod(y, 16) != 0) continue;

		upLeft.position = { 0,                         y * 16.f };
		upRight.position = { downRightScrTile.x * 16.f, y * 16.f };
		downRight.position = { downRightScrTile.x * 16.f, y * 16.f + cam.zoomFactor };
		downLeft.position = { 0,                         y * 16.f + cam.zoomFactor };
		horGrid.append(upLeft);
		horGrid.append(upRight);
		horGrid.append(downRight);
		horGrid.append(downLeft);
	}
	for (int x = upLeftScrTile.x; x < downRightScrTile.x; x++) {
		if (std::fmod(x, 16) != 0) continue;

		upLeft.position = { x * 16.f,                  0 };
		upRight.position = { x * 16.f + cam.zoomFactor, 0 };
		downRight.position = { x * 16.f + cam.zoomFactor, downRightScrTile.y * 16.f };
		downLeft.position = { x * 16.f,					 downRightScrTile.y * 16.f };
		horGrid.append(upLeft);
		horGrid.append(upRight);
		horGrid.append(downRight);
		horGrid.append(downLeft);
	}
	gameWindow.draw(horGrid);
}






void im_render(sf::RenderWindow & window);


void render(sf::RenderWindow & window, sf::Time deltaTime, int framesPerSecond) {
	using namespace sf;

	double deltaFactor = deltaTime.asSeconds() / (1.0 / 60.0);

	window.clear(Color::Black);
	gameWindow.clear(Color::Black);
	debugDrawCalls = 0;
	debugDrawnVertices = 0;
	
	

	Vector2i upLeftScrTile = (Vector2i(cam.position) - (Vector2i(Vector2f(gameWindow.getSize()) * cam.zoomFactor) / 2)) / 16; //Upper left tile on screen
	Vector2i downRightScrTile = (Vector2i(cam.position) + (Vector2i(Vector2f(gameWindow.getSize()) * cam.zoomFactor) / 2)) / 16 + Vector2i(1, 1); //Lower right tile on screen

	upLeftScrTile.x = std::max((int)upLeftScrTile.x, 0);
	upLeftScrTile.y = std::max((int)upLeftScrTile.y, 0);

	downRightScrTile.x = std::min((int)downRightScrTile.x, 64 * 16);
	downRightScrTile.y = std::min((int)downRightScrTile.y, 8 * 16);

	//NOTE: Fix rendering oddities at chunk edges

	RectangleShape backdrop;
	backdrop.setPosition(0, 0);
	backdrop.setFillColor(Color(255, 255, 255, 50));
	backdrop.setSize(Vector2f(64.f, 8.f));
	backdrop.setScale(256.f, 256.f);
	gameWindow.draw(backdrop);

	for (int l = 0; l < currentRoom.chunks.size(); l++) { //Layer
		for (int x = upLeftScrTile.x; x < downRightScrTile.x; x++) {
			for (int y = upLeftScrTile.y; y < downRightScrTile.y; y++) {
				Vector2i currentChunk = { x / 16, y / 16 };
				if (emptychunks[x][y] == CHUNK_EMPTY) {
					y += 16;
					if (y >= downRightScrTile.y)
						break;
				}
				

				WorldTile* currentTile = GetTileAtGlobalIndex(Vector2i(x, y), l, &currentRoom);

				bool nonempty = AddToDrawBuffer(*currentTile, drawBuffers, gameWindow);
				if (emptychunks[x][y] == CHUNK_UNKNOWN && nonempty) {
					std::cout << "Set chunk to nonempty.";
					emptychunks[x][y] = CHUNK_NONEMPTY;
				}
			}
		}
	}
	for (auto& l : currentRoom.objects) {
		for (auto& currentObject : l) {
			for (auto& component : *currentObject.GetComponentsPtr()) {
				gameWindow.draw(component);
				debugDrawCalls++;
			}
		}
	}


	for (auto& b : drawBuffers) {
		gameWindow.draw(b.second, &currentRoom.textures[b.first]);
		debugDrawCalls++;
		debugDrawnVertices += b.second.getVertexCount();
		b.second.clear();
	}

	for (auto selectedTile : selectedTiles) {
		
			RectangleShape tileDataDisplay;
			tileDataDisplay.setFillColor(Color(0, 0, 255, 100));
			float x = selectedTile->chunk.x * 256 + selectedTile->inChunkPos.x * 16;
			float y = selectedTile->chunk.y * 256 + selectedTile->inChunkPos.y * 16;

			for (int i = 0; i < 16; i++) {
				Vector2f pos = Vector2f(x + i, y);
				tileDataDisplay.setPosition(pos + Vector2f(0, 16 - selectedTile->GetHeight(pos, 0)));
				tileDataDisplay.setSize(Vector2f(1, selectedTile->GetHeight(pos, 0)));
				gameWindow.draw(tileDataDisplay);
				debugDrawCalls++;
			}
	}

	//Draw grid
	if (cam.zoomFactor <= 2)
		drawGrid(gameWindow, upLeftScrTile, downRightScrTile);



	if (selectedObject) {
		FloatRect selectRect = selectedObject->getOutline();
		RectangleShape outline;
		outline.setPosition(selectRect.left, selectRect.top);
		outline.setSize({ selectRect.width, selectRect.height });
		outline.setFillColor(Color::Transparent);
		outline.setOutlineColor(Color::Cyan);
		outline.setOutlineThickness(1);
		gameWindow.draw(outline);
		debugDrawCalls++;
	}


	if (tool == Tools::Pencil || tool == Tools::Brush) {
		Sprite tilePlacePreview;
		Vector2i mousePos = Vector2i(hcMouseToWorld(Mouse::getPosition(window), cam, gameWindow.getSize()));
		
		if (mousePos.x >= 0 && mousePos.y >= 0) {
			mousePos /= 16;
			mousePos -= Vector2i(1, 1);

			mousePos *= 16;
			tilePlacePreview.setPosition(Vector2f(mousePos));

			tilePlacePreview.setTexture(currentRoom.textures[currentTexture]);
			Vector2i raw_previewTopLeft = (tileDrawIndex - Vector2i(1, 1)) * 16;
			Vector2i previewTopLeft;
			previewTopLeft.x = std::max(raw_previewTopLeft.x, 0);
			previewTopLeft.y = std::max(raw_previewTopLeft.y, 0);

			Vector2i textureSize = Vector2i(currentRoom.textures[currentTexture].getSize());
			//previewTopLeft.x = std::min(previewTopLeft.x, textureSize.x - 1);
			//previewTopLeft.y = std::min(previewTopLeft.y, textureSize.y - 1);

			Vector2i previewSize = Vector2i(48, 48);
			if (previewTopLeft.x + 48 > textureSize.x)
				previewSize.x = 32;
			if (previewTopLeft.y + 48 > textureSize.y)
				previewSize.y = 32;

			tilePlacePreview.setTextureRect(
				IntRect(
					previewTopLeft,
					previewSize
				)
			);
			tilePlacePreview.setPosition(
				tilePlacePreview.getPosition() + Vector2f(previewTopLeft - raw_previewTopLeft));
			tilePlacePreview.setColor(Color(255, 255, 255, previewOpacity));
			gameWindow.draw(tilePlacePreview);


			tilePlacePreview.setTextureRect(IntRect(tileDrawIndex * 16, Vector2i(16, 16)));
			tilePlacePreview.setPosition(Vector2f(mousePos + Vector2i(16, 16)));
			tilePlacePreview.setColor(Color(255, 255, 255, 200));

			gameWindow.draw(tilePlacePreview);

		}
	}
	if (previewOpacity > 0)
		previewOpacity -= 1.5 * deltaFactor;
	
		

	//cam.position.x = std::max(cam.position.x, window.getSize().x  / (2 * cam.zoomFactor));
	//cam.position.y = std::max(cam.position.y, window.getSize().y / (2 * cam.zoomFactor));
	cam.position.x = std::min(cam.position.x, 256 * 64 - window.getSize().x / (2 * cam.zoomFactor));
	cam.position.y = std::min(cam.position.y, 256 * 8 - window.getSize().y / (2 * cam.zoomFactor));

	Vector2f roundedCamPos = Vector2f(Vector2i(cam.position));

	gameWindow.setView(View(roundedCamPos, Vector2f(Vector2i((Vector2f(window.getSize()) / 2.f) * cam.zoomFactor))));
	gameWindow.display();
	Texture gameWindowTexture = gameWindow.getTexture();
	
	
	Sprite gameWindowSprite;
	gameWindowSprite.setTexture(gameWindowTexture);
	gameWindowSprite.setScale(Vector2f(2, 2));

	//ImGui::SetNextWindowPos(ImVec2(0, 0));
	//ImGui::SetNextWindowSize(ImVec2(window.getSize().x, window.getSize().y));
	


	window.draw(gameWindowSprite);
	
	
	
	
	
	Text debugInfo;
	debugInfo.setFillColor(Color::White);
	debugInfo.setFont(OpenSans);
	debugInfo.setCharacterSize(15);
	
	using namespace std;


	string debugString = "\nHedgehogCreator - By Robin A. Bellamy\nselected: { ";

	for (auto s : selectedTiles) {
		string sAddr = AddressStr((void const*)s);

		debugString += "0x" + sAddr + ", ";
	}
	debugString = debugString.substr(0, debugString.size() - 2) + " }\n";

	debugString += "tool: "					+ to_string(tool) + "\n";
	debugString += "drawcalls: "			+ to_string(debugDrawCalls) + "\n";
	debugString += "drawn tile vertices: "	+ to_string(debugDrawnVertices) + "\n";
	debugString += "ms: "					+ to_string(deltaTime.asMilliseconds()) + "\n";
	debugString += "fps: "					+ to_string(1.f / deltaTime.asSeconds()) + "\n";
	debugString += "real window size: "		+ to_string(window.getSize().x) + ", " + to_string(window.getSize().y) + "\n";
	debugString += "render window size: "	+ to_string(gameWindow.getSize().x) + ", " + to_string(gameWindow.getSize().y) + "\n";
	debugString += "render view size: "		+ to_string(gameWindow.getView().getSize().x) + ", " + to_string(gameWindow.getView().getSize().y) + "\n";
	if (sf::Vector2f(gameWindow.getSize()) == gameWindow.getView().getSize()) {
		debugString += "no discrepancy";
	}
	else {
		debugString += "discrepancy of " +
			to_string(gameWindow.getView().getSize().x - gameWindow.getSize().x) + ", " +
			to_string(gameWindow.getView().getSize().y - gameWindow.getSize().y) + "\n";

	}
	debugInfo.setString(debugString);
	//debugInfo.setPosition(gameWindowSprite.getPosition());
	window.draw(debugInfo);


	im_render(window);



}

namespace menu
{
	bool projectWindow = true;
	bool toolsWindow = true;
	bool objectsWindow = true;
	bool parallaxWindow = true;
	bool playerWindow = true;
}

std::string GetAssetPath(std::string assetfilename) {
	return GetOwnFilePath() + "/Assets/" + assetfilename;
}

void im_render(sf::RenderWindow& window) {
	using namespace sf;
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{

			if (ImGui::MenuItem("New")) { newRoom(); }
			if (ImGui::MenuItem("Open")) { openRoom(); }
			if (ImGui::MenuItem("Save")) { saveRoom(); }
			ImGui::Separator();
			if (ImGui::BeginMenu("Import...")) {
				if (ImGui::MenuItem("Tilemap")) {
					const char* filter[] = { "*.png", "*.jpg", "*.bmp", "*.tga", "*.gif", "*.psd" };
					char const* input = tinyfd_openFileDialog("Open...", (GetOwnFilePath() + "/Projects/").c_str(), 1, &filter[0], "", 0);
					if (input != NULL) {
						
						std::string pathname(input);
						std::string filename(pathname.substr(pathname.find_last_of("\\") + 1, pathname.size() - pathname.find_last_of("\\") - 1));
						
						copyFile(pathname.c_str(), (GetAssetPath(filename).c_str()));
						currentRoom.AddTileMap(filename);
					}
				}
				if (ImGui::MenuItem("Texture")) {
					const char* filter[] = { "*.png", "*.jpg", "*.bmp", "*.tga", "*.gif", "*.psd" };
					char const* input = tinyfd_openFileDialog("Open...", (GetOwnFilePath() + "/Projects/").c_str(), 1, &filter[0], "", 0);
					if (input != NULL) {
						std::string pathname(input);
						std::string filename(pathname.substr(pathname.find_last_of("\\") + 1, pathname.size() - pathname.find_last_of("\\") - 1));
						

						fs::path dir(GetAssetPath(""));

						if (!(fs::exists(dir)))
							fs::create_directory(dir);

						copyFile(pathname.c_str(), (GetAssetPath(filename).c_str()));
						sf::Texture txt;
						txt.loadFromFile(GetAssetPath(filename));

						currentRoom.textures.insert(std::pair<std::string, sf::Texture>(filename, txt));
					}
				}
				if (ImGui::MenuItem("Audio")) {
					const char* filter[] = { "*.wav", "*.ogg", "*.flac" };
					char const* input = tinyfd_openFileDialog("Open...", (GetOwnFilePath() + "/Projects/").c_str(), 1, &filter[0], "", 0);
					if (input != NULL) {
						std::string pathname(input);
						std::string filename(pathname.substr(pathname.find_last_of("\\") + 1, pathname.size() - pathname.find_last_of("\\") - 1));

						copyFile(pathname.c_str(), (GetAssetPath(filename).c_str()));
						sf::SoundBuffer sound;
						sound.loadFromFile((GetOwnFilePath() + "/" + currentRoom.name + "_Assets/" + filename));
						currentRoom.sounds.insert(std::pair<std::string, sf::SoundBuffer>(filename, sound));
					}
				}
				ImGui::EndMenu();
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit"))
		{
			if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
			if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
			ImGui::Separator();
			if (ImGui::MenuItem("Cut", "CTRL+X")) {}
			if (ImGui::MenuItem("Copy", "CTRL+C")) {}
			if (ImGui::MenuItem("Paste", "CTRL+V")) {}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("View")) {
			ImGui::MenuItem("Objects", "", &menu::objectsWindow);
			ImGui::MenuItem("Parallax Layers", "", &menu::parallaxWindow);
			ImGui::MenuItem("Player", "", &menu::playerWindow);
			ImGui::MenuItem("Tools", "", &menu::toolsWindow);
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Debug")) {
			ImGui::InputInt("Tool", &tool);
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Run")) {

			std::wstring path(L"" + GetOwnFilePath() + L"/HedgehogCore.exe");
			std::wstring command(L"-r:" + currentRoom.path + currentRoom.filename);

			if (ImGui::MenuItem("Current room")) {
				saveRoom();
				std::cout << "Starting HedgehogCore...\n\n";
				ShellExecute(GetConsoleWindow(), L"open", path.c_str(), command.c_str(), L"", SW_HIDE);
			}
			if (ImGui::MenuItem("Current room (Don't rebuild)")) {
				std::cout << "Starting HedgehogCore... (no rebuild)\n\n";
				ShellExecute(GetConsoleWindow(), L"open", path.c_str(), command.c_str(), L"", SW_HIDE);
			}


			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Help")) {

			if (ImGui::MenuItem("About...")) {
				RenderWindow aboutWind(VideoMode(800, 400), "About HedgehogCreator", Style::Close);
				bool aboutWindRunning = true;
				while (aboutWindRunning) {
					Event event;
					while (aboutWind.pollEvent(event)) {
						if (event.type == Event::Closed) {
							aboutWindRunning = false;
						}
					}
					aboutWind.clear(Color(0.06f * 255, 0.05f * 255, 0.07f * 255, 1.00f * 255));
					Text aboutInfo;
					aboutInfo.setPosition(4, 4);
					aboutInfo.setFillColor(Color::White);
					aboutInfo.setFont(OpenSans);
					aboutInfo.setCharacterSize(20);

					using namespace std;
					aboutInfo.setString(
						"HedgehogCreator v" + to_string(VERSION) +
						"\nBy Robin Bellamy \nUsed libraries: \n -SFML 2.5.1 \n -Dear ImGUI \n -OpenGL \n -tinyfiledialog \n -SelbaWard\n\n");
					aboutWind.draw(aboutInfo);
					aboutWind.display();
				}
			}

			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
	
	if (menu::parallaxWindow) {
		ImGui::Begin("Parallax Layers");
		ImGui::End();
	}

	if (menu::objectsWindow) {
		ImGui::Begin("Objects"); // begin window
		if (ImGui::Button("New Object")) {
			/*int objcount = 500;
			uint64 pretime = GetTimeMs64();
			for (int i = 0; i < objcount; i++) {
			HCObject *obj = NewObject({ NewScript(&domain, "Spring") }, ("Test #" + std::to_string(i)));
			obj->startposition = Vector2f(i * 256, 256);
			obj->position = obj->startposition;
			}
			std::cout << "Mass init benchmark (" << objcount << " objects): " << GetTimeMs64() - pretime << "ms\n";*/
			//globalObjInitCount++;
			HCObject* obj = NewObject({}, currentLayer, &currentRoom, ("New Object #" + std::to_string(0)));
			obj->startposition = cam.position;
			obj->position = obj->startposition;
		}

		std::vector<std::string> names;
		std::vector<HCObject*> pointers;




		for (int l = 0; l < currentRoom.objects.size(); l++) {
			for (int i = 0; i < currentRoom.objects[l].size(); i++) {
				names.push_back(currentRoom.objects[l][i].name.c_str());
				pointers.push_back(&currentRoom.objects[l][i]);
			}
		}
		static int listbox_item_current = 1;
		if (ImGui::ListBox("", &listbox_item_current, names)) {
			selectedObject = pointers[listbox_item_current];
		}


		if (selectedObject) {
			ImGui::Separator();
			
			float pos[2] = { selectedObject->position.x, selectedObject->position.y };
			ImGui::InputFloat2("Position", pos, 1.0f);
			selectedObject->position = { pos[0], pos[1] };

			float size[2] = { selectedObject->GetSize().x, selectedObject->GetSize().y };
			ImGui::InputFloat2("Size", size, 1.0f);
			selectedObject->SetSize({size[0], size[1]});


			for (auto& component : *selectedObject->GetComponentsPtr()) {
				component.im_draw();
				ImGui::Separator();
				gameWindow.draw(component);
			}
			if (ImGui::Button("New Component")) {
				ImGui::OpenPopup("Select asset##add component");
			}

			if (ImGui::BeginPopup("Select asset##add component"))
			{
				int index;
				std::vector<std::string> components = { "SpriteRenderer", "Renderer3D", "SheetAnimator" };
				if (ImGui::ListBox("Type", &index, components)) {
					switch (index) {
					case 0:
						selectedObject->AddComponent(new SpriteRenderer(selectedObject, &currentRoom));
						break;
					case 1:
						selectedObject->AddComponent(new Renderer3D(selectedObject, &currentRoom));
						break;
					case 2:
						selectedObject->AddComponent(new SheetAnimator(selectedObject, &currentRoom));
					default:
						break;
					}
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}
			

		}

		ImGui::End(); // end window
	}


	/*if (menu::playerWindow) {
		ImGui::Begin("Player"); // begin window

		ImGui::Text("\nPosition");
		ImGui::InputFloat("X", &player.state.position.x, 1);
		ImGui::InputFloat("Y", &player.state.position.y, 1);
		ImGui::Text("\nVelocity");
		ImGui::InputFloat("XSPD", &player.state.speed.x, 1);
		ImGui::InputFloat("YSPD", &player.state.speed.y, 1);
		ImGui::InputDouble("GSPD", &player.state.groundSpeed, 1);

		ImGui::End(); // end window
	}*/
	if (menu::toolsWindow) {
		ImGui::Begin("Tools"); // begin window
		
		ImGui::Text("\nTool");
		std::vector<std::string> tooloptions = { "Select", "Pencil", "Brush", "Flood Fill", "Eraser" };
		ImGui::Combo(&"      "[0], &tool, tooloptions);
		ImGui::Text("Layer");
		ImGui::InputInt("       ", &currentLayer, 1, 2);

		
		ImGui::Text(("Current tilemap: " + currentTexture).c_str());
		if (ImGui::Button("Change")) {
			assetselectors.insert(std::pair<std::string, AssetSelector>(TILEMAPSELECTOR_TOKEN, AssetSelector(&currentRoom, AssetType::Tilemap)));
		}
		if (assetselectors.count(TILEMAPSELECTOR_TOKEN) > 0) {
			assetselectors[TILEMAPSELECTOR_TOKEN].Update(1);
			if (assetselectors[TILEMAPSELECTOR_TOKEN].asset.state == 1) {
				currentTexture = assetselectors[TILEMAPSELECTOR_TOKEN].asset.path;
				assetselectors.erase(TILEMAPSELECTOR_TOKEN);
			}
			else if (assetselectors[TILEMAPSELECTOR_TOKEN].asset.state == -1) {
				assetselectors.erase(TILEMAPSELECTOR_TOKEN);
			}
		}

		if (selectedTiles.size() > 0) {
			ImGui::Separator();
			ImGui::Text("Selected");
			bool sameTile = true;
			for (int i = 1; i < selectedTiles.size(); i++) {
				WorldTile& currentTile = *selectedTiles[i];
				if (currentTile.tile != selectedTiles[i - 1]->tile) {
					sameTile = false;
					break;
				}
			}
		}
		ImGui::End(); // end window
	}
}