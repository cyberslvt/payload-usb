#pragma once

#include <unordered_map>
#include <string>
#include <fstream>
#include <cassert>
#include <filesystem>

#include "../utils/string.h"

class Configuration
{
private:
	std::unordered_map<std::string, std::string> m_map;

private:
	bool ParseLine(const std::string& line);
public:
	static bool LoadFromFile(const std::string& filepath, Configuration& out_config);
	static bool AppendFromFile(const std::string& filepath, Configuration& base_config);

	std::optional<std::string> GetValue(const std::string& key);
};

inline bool Configuration::ParseLine(const std::string& line) {
	if (line.find("//") != std::string::npos) { return false; }
	uint64_t splitPos = line.find("=");
	if (splitPos == std::string::npos) { return false; }

	std::string key = line.substr(0, splitPos);
	std::string val = line.substr(splitPos + 1);
	key = Utility::String::trim(key);
	val = Utility::String::trim(val);
	m_map.insert({ key, val });
	return true;
}

inline bool Configuration::LoadFromFile(const std::string& filepath, Configuration& out_config) {
	if (!std::filesystem::exists(filepath)) { return false; }
	Configuration newConfig;

	std::ifstream f(filepath);

	std::string line;
	while (std::getline(f, line))
	{
		newConfig.ParseLine(line);
	}

	out_config = newConfig;
	return true;
}

inline bool Configuration::AppendFromFile(const std::string& filepath, Configuration& base_config) {
	if (!std::filesystem::exists(filepath)) { return false; }

	std::ifstream f(filepath);

	std::string line;
	while (std::getline(f, line))
	{
		base_config.ParseLine(line);
	}

	return true;
}

inline std::optional<std::string> Configuration::GetValue(const std::string& key)
{
	if (!m_map.contains(key)) { return std::nullopt; }
	std::string keyc = key;
	keyc = Utility::String::trim(keyc);
	return m_map[keyc];
}