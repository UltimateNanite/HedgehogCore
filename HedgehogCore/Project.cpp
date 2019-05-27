#include "stdafx.h"
#include "Project.h"
#include <filesystem>
namespace fs = std::filesystem;
Project::Project()
{
}


Project::~Project()
{
}

std::ofstream &operator<<(std::ofstream& ofs, Project& rt) {
	float version = VERSION;//rt.GetVersionData().GetVersion();
	ofs << version << std::endl;
	ofs << rt.GetRoomsPtr()->size() << std::endl;
	for (int i = 0; i < rt.GetRoomsPtr()->size(); i++) {
		std::string filepath = GetCurrentProjectPath(rt) + "/Rooms/";
		fs::path dir(filepath);
		if (!(fs::exists(dir))) {
			std::cout << "Creating path\n";
			std::error_code ec;

			if (fs::create_directory(dir, ec))
				std::cout << "Path created.\n";
			else
				std::cout << "Failed to create path, error code " << ec.message() << "\n";
		}
		if (rt.GetRoomsPtr()->at(i).name == "")
			rt.GetRoomsPtr()->at(i).name = "Untitled Room";
		std::ofstream fileout(std::string(filepath + rt.GetRoomsPtr()->at(i).name + ".room").c_str());
		std::cout << "Saving room \"" << rt.GetRoomsPtr()->at(i).name << "\" to path " << filepath << std::endl;
		fileout << rt.GetRoomsPtr()->at(i);
		fileout.close();

		ofs << rt.GetRoomsPtr()->at(i).name.c_str() << std::endl;
	}
	
	return ofs;
}
std::ifstream &	operator>>(std::ifstream& ifs, Project *rt) {
	rt->GetRoomsPtr()->clear();
	while (ifs.good()) {
		float fileVers = 0;
		int roomsSize = 0;
		ifs >> fileVers >> roomsSize;
		if (fileVers <= VERSION) {
			if (fileVers < VERSION) {
				//Warn about conversion from old to new
			}

			for (int i = 0; i < roomsSize; i++) {
				Room roomInit(rt);
				rt->GetRoomsPtr()->push_back(roomInit);
				Room &room = rt->GetRoomsPtr()->at(rt->GetRoomsPtr()->size() - 1);
				while (room.name == "") {
					std::getline(ifs, room.name);
				}
				std::string filepath = GetCurrentProjectPath(*rt) + "/Rooms/";
				fs::path dir(filepath);
				if (!(fs::exists(dir))) {
					std::cout << "Creating path " + filepath + "\n";
					std::error_code ec;

					if (fs::create_directory(dir, ec))
						std::cout << "Path created.\n";
					else
						std::cout << "Failed to create path, error code " << ec.message() << "\n";
				}

				std::ifstream filein(std::string(filepath + room.name + ".room").c_str());

				filein >> room;
				
			}
		}
		else {
			//Newer, can't open
		}
	}
	rt->path = GetOwnFilePath() + "/Projects/" + rt->name + "/";
	return ifs; //Texture references become invalid when going out of scope
}
void Project::UnloadAssets() {
	scriptnames.clear();
	texturenames.clear();
	audionames.clear();
}
void Project::LoadAssets() {
	
	for (auto& p : fs::recursive_directory_iterator(this->path + "Assets\\")) {
		std::cout << p.path() << '\n';
		std::string pS = p.path().string();
		if (pS.find("Assets\\") != -1) {
			pS = pS.substr(pS.find("Assets\\") + 7);
			std::string ext = pS.substr(pS.find_last_of("."));
			if (ext == ".cs")
				scriptnames.push_back(pS.substr(0, pS.find_last_of(".")));
			else if (ext == ".png" || ext == ".bmp" || ext == ".tga" || ext == ".jpg" || ext == ".psd" || ext == ".hdr" || ext == ".pic")
				texturenames.push_back(pS);
			else if (ext == ".wav" || ext == ".ogg" || ext == ".flac")
				audionames.push_back(pS.substr(0, pS.find_last_of(".")));
			else if (ext == ".tdata") {
				//Do nothing, AddTileMap() handles tdata
			}
			else
				std::cout << "Unrecognized filetype: " << ext << " (" << p.path() << ")\n";
		}
	}
}
bool Project::Load(std::string filepath) {
	std::ifstream filein(std::string(filepath).c_str());
	UnloadAssets();
	LoadAssets();
	
	filein >> this;
	filein.close();
	return true;
}
bool Project::LoadWithFileSelector() { return false;  }
bool Project::Save(std::string filepath) {
	fs::path dir(filepath);
	if (!(fs::exists(dir))) {
		std::cout << "Creating path " + filepath + "\n";
		std::error_code ec;

		if (fs::create_directory(dir, ec))
			std::cout << "Path created.\n";
		else {
			std::cout << "Failed to create path, error code " << ec.message() << "\n";
			return false;
		}
	}
	filepath += "/" + this->name + ".skies";

	std::ofstream fileout(filepath.c_str());
	fileout << *this;
	fileout.close();
	return true;
}
bool Project::SaveWithFileSelector() { return false; }

boost::container::stable_vector<Room> *Project::GetRoomsPtr() {
	return &this->rooms;
}