#pragma once
#include "Includes.h"
#include "Utilities.h"
#include "Room.h"

class Project
{
private:
	boost::container::stable_vector<Room> rooms;
public:
	//std::map <std::string, fs::v1::file_time_type> scripttimes;
	bool Load(std::string filepath);
	bool LoadWithFileSelector();
	bool Save(std::string filepath = "");
	bool SaveWithFileSelector();
	Project();
	std::vector<std::string> scriptnames = { "Spring" };
	std::vector<std::string> texturenames;
	std::vector<std::string> audionames;
	void UnloadAssets();
	void LoadAssets();
	std::string path;
	boost::container::stable_vector<Room> *GetRoomsPtr();
	std::string name = "New Project";
	~Project();
};

