#include "stdafx.h"
#include "Room.h"
#include "Project.h"
#include "Structs.h"
#include <filesystem>
namespace fs = std::filesystem;
TextureRefInfo WorldTileToTextureRefInfo(WorldTile tile) {
	if (!tile.tile) return TextureRefInfo("", -1, -1);
	return tile.tile->GetRefInfo();
}
WorldTile TextureRefToWorldTile(TextureRefInfo ref, sf::Vector2i tileIndex, Room *currentRoom) {
	WorldTile result;
	result.tile = &currentRoom->tilemapdata[ref.name][ref.textureRectIndex.x][ref.textureRectIndex.y];
	result.SetPosition(tileIndex);
	return result;
}

Room::Room(Project *parent)
{
	this->name = "New Room";
	this->currentProject = parent;
	chunks.resize(2);
	objects.resize(2);
	for (int l = 0; l < 2; l++) {
		chunks[l].resize(64);
		for (int x = 0; x < 64; x++) {
			chunks[l][x].resize(8);
			for (int y = 0; y < 8; y++) {
				chunks[l][x][y].resize(16);
				for (int i = 0; i < 16; i++) {
					chunks[l][x][y][i].resize(16);
					for (int t = 0; t < chunks[l][x][y][i].size(); t++) {
						WorldTile &currentTile = chunks[l][x][y][i][t];
						currentTile.chunk = sf::Vector2<unsigned short>(x,y);
						currentTile.inChunkPos = sf::Vector2<unsigned short>( i,t );
					}
				}
			}
		}
	}
}

Room::Room()
{
}


Room::~Room()
{
}

#define TILEMAPID -2
#define TEXTUREID -3
#define SOUNDID -4
#define ROOM_SAVE_TYPE ""


std::ofstream& operator<<(std::ofstream& ofs, const Room& rt) {

	for (auto s : rt.tilemapnames) {
		ofs << TILEMAPID << " " << s << std::endl;
	}
	for (int l = 0; l < rt.chunks.size(); l++) { //Layer
		for (int cx = 0; cx < rt.chunks[l].size(); cx++) {
			for (int cy = 0; cy < rt.chunks[l][cx].size(); cy++) {
				for (int x = 0; x < 16; x++) {
					for (int y = 0; y < 16; y++) {
						TextureRefInfo currentTileRef = WorldTileToTextureRefInfo(rt.chunks[l][cx][cy][x][y]);
						if (currentTileRef.name != "") {
							ofs << l << ":" << cx << ":" << cy << ":" << x << ":" << y << ":" << 0 << currentTileRef.name << '|' << "," << currentTileRef.textureRectIndex.x << "," << currentTileRef.textureRectIndex.y << ".";
						}
						
					}
				}
			}
		}
	}
	ofs << -1; //End of Tiles

	//Save tile data to .tdata files
	for (auto& it : rt.tilemapdata) {
		std::string filepath = it.first + ".tdata";
		std::ofstream fileout(filepath.c_str());
		fileout << it.second;
	}

	for (auto& obj : rt.objects) {
		
	}
	return ofs;
}

std::ifstream& operator>>(std::ifstream& ifs, Room& rt) {

	
	std::cout << "Loading room \"" << rt.name << "\"\n";
	rt.chunks = Room(nullptr).chunks;

	if (!ifs.good()) throw std::exception("Bad filestream.");
	while (ifs.good()) {
		
		//Tile refs
		int l = 0;
		int cx = 0;
		int cy = 0;
		int x = 0;
		int y = 0;
		char del1 = '\0', del2 = '\0', del3 = '\0', del4 = '\0', del5 = '\0', del6 = '\0', del7 = '\0', del8 = '\0';
		TextureRefInfo currentTileRef("",0,0);
		ifs >> l;
		if (l < 0) {
			
			if (l == -1) {
				ifs >> del1;
				continue;
			}
			if (l == TILEMAPID) {
				
				std::string tilemapname;
				ifs >> tilemapname;
				std::cout << "Adding tilemap " << tilemapname << std::endl;
				//ifs >> del1;
				rt.AddTileMap(tilemapname);
				
				continue;
			}
		}
		int delim = 0;
		ifs >> del1 >> cx >> del2 >> cy >> del3 >> x >> del4 >> y >> del5 >> delim;

		char namein = '\0';
		int nametimeout = 1000;
		while (nametimeout > 0) {
			nametimeout--;
			ifs >> namein;
			if (namein == '|')
				break;
			currentTileRef.name += namein;
		}
		if (nametimeout == 0)
			throw std::runtime_error("Room file corrupted.");
		
		ifs >> del6 >> currentTileRef.textureRectIndex.x >> del7 >> currentTileRef.textureRectIndex.y >> del8;
		rt.chunks[l][cx][cy][x][y] = TextureRefToWorldTile(currentTileRef, sf::Vector2i(0, 0), &rt);
		rt.chunks[l][cx][cy][x][y].chunk = sf::Vector2<unsigned short>(cx, cy);
		rt.chunks[l][cx][cy][x][y].inChunkPos = sf::Vector2<unsigned short>(x, y);
		if (x == -1 || ifs.eof()) break;

			


	}
	
	return ifs;
}

void Room::AddTileMap(std::string tilemapname, bool forceRecreate) {
	this->tilemapnames.push_back(tilemapname);
	sf::Texture txt;
	std::string tilemappath = path + tilemapname;
	if (!txt.loadFromFile(tilemappath)) {
		throw std::runtime_error("Error loading " + tilemapname);
		return;
	}
	

	this->textures.insert(std::pair<std::string, sf::Texture>(tilemapname,txt));
	
	fs::path dir(tilemappath + ".tdata");
	if (fs::exists(dir) && !forceRecreate) {
		std::ifstream ifs(dir);
		TileData td;
		ifs >> td;
		Tile::SetTileDataFilename(td, tilemapname);
		Tile::SetTxtsPointer(td, this);
		this->tilemapdata.insert(std::pair < std::string, TileData>(tilemapname, td));
		return;
	}

	std::vector<std::vector<Tile>> tileSet;
	tileSet.resize(txt.getSize().x / 16);
	sf::Image img = txt.copyToImage();
	for (int x = 0; x < txt.getSize().x / 16; x++) {
		tileSet[x].resize(txt.getSize().y / 16);
		for (int y = 0; y < txt.getSize().y / 16; y++) {
			Tile result;
			result.txts = &textures;
			result.texturename = tilemapname;
			result.textureRectIndex = sf::Vector2<unsigned short>(x, y);
			result.heightArrays.down = GenerateHeightArray(img, sf::Vector2i(x, y), 0);
			result.heightArrays.right = GenerateHeightArray(img, sf::Vector2i(x, y), 3);
			result.heightArrays.up = GenerateHeightArray(img, sf::Vector2i(x, y), 2);
			result.heightArrays.left = GenerateHeightArray(img, sf::Vector2i(x, y), 1);
			tileSet[x][y] = result;
		}
	}
	this->tilemapdata.insert(std::pair < std::string, TileData>(tilemapname, tileSet));
}

std::ofstream& operator<<(std::ofstream& ofs, const TileData& td) {
	
	for (int x = 0; x < td.size(); x++) {
		for (int y = 0; y < td[x].size(); y++) {
			Tile tile = td[x][y];
			ofs << x << "," << y << ":";
			TileHeightInfo heights = tile.heightArrays;
			std::vector<std::vector<unsigned char>> data = { heights.down, heights.left, heights.up, heights.right };
			ofs << "[";
			for (auto heightData : data) {
				ofs << "{";
				for (int i = 0; i < heightData.size(); i++) {
					ofs << (int)heightData[i];
					if (i < heightData.size() - 1) 
						ofs << ",";
				}
				ofs << "}";
			}
			ofs << "]";
			
		}
	}
	return ofs;
}





std::ifstream& operator>>(std::ifstream& ifs, TileData& td) {
	while (ifs.good()) {
		int x = 0;
		int y = 0;
		char del1 = '\0', del2 = '\0', del3 = '\0', del4 = '\0';
		ifs >> x >> del1 >> y >> del2 >> del3 >> del4;
		Tile tile;
		if (del1 != ',')
			break;
		for (int it = 0; it < 4; it++) {
			int i = 0;
			std::vector<unsigned char> hData;
			TileHeightInfo& heightArrays = tile.heightArrays;
			while (del4 != '}') {
				ifs >> i >> del4;
				hData.push_back(i);
				if (del4 != ',' && del4 != '}')
					break;
			}
			ifs >> del4;
			tile.textureRectIndex = { (unsigned short)x, (unsigned short)y };
			
			if (it == 0)
				heightArrays.down = hData;
			if (it == 3)
				heightArrays.right = hData;
			if (it == 2)
				heightArrays.up = hData;
			if (it == 1)
				heightArrays.left = hData;
		}
		if (td.size() >= x) {
			td.resize(x + 1);
		}
		if (td[x].size() >= y) {
			td[x].resize(y + 1);
		}
		td[x][y] = tile;

		if (ifs.eof()) 
			break;
	}
	return ifs;
}