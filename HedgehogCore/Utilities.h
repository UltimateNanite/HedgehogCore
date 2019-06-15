#pragma once
#include <stdlib.h>
#include <vector>
#include <string>



class Project;
class Room;
struct WorldTile;
struct TextureRefInfo;
#include "Includes.h"
float Rad(float a);
float Lerp(float a, float b, float f);
int GetSign(float x);
float LerpDegrees(float a, float b, float f);
long RoundDown(long n, long m);
long RoundUp(long n, long m);
std::string GetOwnFilePath();
std::string AddressStr(void const* address);
sf::Vector2f operator*(sf::Vector2f a, sf::Vector2f b);
sf::Vector2f operator/(sf::Vector2f a, sf::Vector2f b);
sf::Vector2f operator*=(sf::Vector2f a, sf::Vector2f b);
sf::Vector2f operator/=(sf::Vector2f a, sf::Vector2f b);
sf::Vector2i operator*(sf::Vector2i a, sf::Vector2i b);
template<typename T, size_t SIZE> inline void zero(T(&arr)[SIZE]) {
	memset(arr, 0, SIZE * sizeof(T));
}
std::basic_string<TCHAR> GetErrorMessage(DWORD dwErrorCode);



TextureRefInfo WorldTileToTextureRefInfo(WorldTile tile);
WorldTile TextureRefToWorldTile(TextureRefInfo ref, sf::Vector2i tileIndex, Room *currentRoom);
bool copyFile(const char *SRC, const char* DEST);


std::string GetCurrentProjectPath(Project &currentProject);

std::string GetFullScriptPath(Project &currentProject, std::string scriptname, bool includecs = true);

std::string GetFullAssemblyPath(Project &currentProject, std::string scriptname);

template<typename T>
T random(T min, T max);
