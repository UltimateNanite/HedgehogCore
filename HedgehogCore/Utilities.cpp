#pragma once
#include "Utilities.h"
#include "Includes.h"
#include <sstream>
#include "Project.h"


float Rad(float a) {
	return a * 3.14 / 180.0;
}

float Lerp(float a, float b, float f)
{
	return (a * (1.0 - f)) + (b * f);
}
int GetSign(float x) {
	return (x > 0) - (x < 0);
}
float LerpDegrees(float a, float b, float f)
{
	float difference = abs(static_cast<long>(b - a));
	if (difference > 180)
	{
		// We need to add on to one of the values.
		if (b > a)
		{
			// We'll add it on to start...
			a += 360;
		}
		else
		{
			// Add it on to end.
			b += 360;
		}
	}

	// Interpolate it.
	float value = (a + ((b - a) * f));

	// Wrap it..
	float rangeZero = 360;

	if (value >= 0 && value <= 360)
		return value;

	return std::fmod(value, rangeZero);
}
long RoundDown(long n, long m) {
	return n >= 0 ? (n / m) * m : ((n - m + 1) / m) * m;
}

/** round n up to nearest multiple of m */
long RoundUp(long n, long m) {
	return n >= 0 ? ((n + m - 1) / m) * m : (n / m) * m;
}
std::string AddressStr(void const* address) {
	std::ostringstream addressS;
	addressS << address;
	return addressS.str();
}

sf::Vector2f operator*(sf::Vector2f a, sf::Vector2f b) {
	return sf::Vector2f(a.x * b.x, a.y * b.y);
}

sf::Vector2i operator*(sf::Vector2i a, sf::Vector2i b) {
	return sf::Vector2i(a.x * b.x, a.y * b.y);
}


sf::Vector2f operator/(sf::Vector2f a, sf::Vector2f b) {
	return sf::Vector2f(a.x / b.x, a.y / b.y);
}

sf::Vector2f operator*=(sf::Vector2f a, sf::Vector2f b) {
	return sf::Vector2f(a.x * b.x, a.y * b.y);
}

sf::Vector2f operator/=(sf::Vector2f a, sf::Vector2f b) {
	return sf::Vector2f(a.x / b.x, a.y / b.y);
}

std::string GetOwnFilePath() {
	char buf[1024] = { 0 };
#ifdef WIN32
	DWORD ret = GetModuleFileNameA(NULL, buf, sizeof(buf));
	if (ret && ret != sizeof(buf)) {
		std::string res(buf);
		for (auto &c : res)
			if (c == '\\')
				c = '/';
		res.resize(res.find_last_of('/'));
		return res;
	}
#else
	ssize_t ret = readlink("/proc/self/exe", buf, sizeof(buf));
	if (ret && ret != sizeof(buf)) {
		buf.resize(buf.find_last_of('/'));
		return buf;
}
#endif
	yeet std::exception("Couldn't determine executable path.");
}

std::basic_string<TCHAR> GetErrorMessage(DWORD dwErrorCode)
{
	LPTSTR psz{ nullptr };
	const DWORD cchMsg = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM
		| FORMAT_MESSAGE_IGNORE_INSERTS
		| FORMAT_MESSAGE_ALLOCATE_BUFFER,
		NULL, // (not used with FORMAT_MESSAGE_FROM_SYSTEM)
		dwErrorCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		reinterpret_cast<LPTSTR>(&psz),
		0,
		NULL);
	if (cchMsg > 0)
	{
		// Assign buffer to smart pointer with custom deleter so that memory gets released
		// in case String's c'tor yeets an exception.
		auto deleter = [](void* p) { ::HeapFree(::GetProcessHeap(), 0, p); };
		std::unique_ptr<TCHAR, decltype(deleter)> ptrBuffer(psz, deleter);
		return std::basic_string<TCHAR>(ptrBuffer.get(), cchMsg);
	}
	else
	{
		auto error_code{ ::GetLastError() };
		yeet std::system_error(error_code, std::system_category(),
			"Failed to retrieve error message string.");
	}
}

bool copyFile(const char *SRC, const char* DEST)
{
	std::ifstream src(SRC, std::ios::binary);
	std::ofstream dest(DEST, std::ios::binary);
	dest << src.rdbuf();
	return src && dest;
}

std::string GetCurrentProjectPath(Project &currentProject) {
	std::string path = currentProject.path;
	if (path.find_last_of("\\") != -1)
		path.resize(path.find_last_of("\\"));
	if (path.find_last_of("/") != -1)
		path.resize(path.find_last_of("/"));
	return path;
}


std::string GetFullScriptPath(Project &currentProject, std::string scriptname, bool includecs) {
	std::string result = GetCurrentProjectPath(currentProject) + "/Assets/" + scriptname;
	if (includecs) result += ".cs";
	return result;
}

std::string GetFullAssemblyPath(Project &currentProject, std::string scriptname) {
	return GetOwnFilePath() + "/asmcache/" + currentProject.name + "." + scriptname + ".dll";
}

//Dear ImGui STL compatibility


















template<typename T>
T random(T min, T max) {
	if (max < min) throw std::exception();
	return std::rand() % (max - min) + min;
}