#include <iostream>
#include <string>
#include <algorithm>
#include <functional>

#include "cfg/cfgparser.h"
#include "utils/string.h"

#define CRITICAL_ASSERT(cond, file, lineNum, msg) if(!cond){std::cout << "Error! File '" << file << "' [Line: " << lineNum << "]: " << msg; std::cin.get(); return 1;}
Configuration keyboardConfig;
Configuration layoutConfig;

void addBytes(std::vector<std::byte>& finalBytes, std::vector<std::byte>& prevBytes, std::vector<std::byte>& captureBytes, bool capture, std::vector<std::byte> bytes) {
	for (int i = 0; i < bytes.size(); i++)
	{
		finalBytes.push_back(bytes[i]);
		prevBytes.push_back(bytes[i]);
		if (capture)
			captureBytes.push_back(bytes[i]);
	}
	if (bytes.size() % 2 != 0) {
		finalBytes.push_back((std::byte)0x00);
		prevBytes.push_back((std::byte)0x00);
		if (capture)
			captureBytes.push_back((std::byte)0x00);
	}
}

std::byte strToByte(std::string str) {
	if (str.starts_with("0x")) {
		return (std::byte)std::stoi(str.substr(2), nullptr, 16);
	}
	else {
		return (std::byte)std::stoi(str);
	}
}

std::vector<std::byte> codeToBytes(std::string str) {
	if (layoutConfig.GetValue(str) != std::nullopt) {
		std::vector<std::string> keys = Utility::String::split(layoutConfig.GetValue(str).value(), ",");
		std::vector<std::byte> bytes = std::vector<std::byte>(keys.size());
		for (int j = 0; j < keys.size(); j++) {
			std::string key = Utility::String::trim(keys[j]);
			if (keyboardConfig.GetValue(key) != std::nullopt) {
				auto val = keyboardConfig.GetValue(key).value();
				bytes[j] = (strToByte(Utility::String::trim(val)));
			}
			else if (layoutConfig.GetValue(key) != std::nullopt) {
				auto val = layoutConfig.GetValue(key).value();
				bytes[j] = (strToByte(Utility::String::trim(val)));
			}
			else {
				std::cout << "Key not found:" + key << std::endl;
				bytes[j] = (std::byte(0x00));
			}
		}
		return bytes;
	}
	else {
		std::cout << "Char not found:" + str << std::endl;
		std::vector<std::byte> bytes = std::vector<std::byte>(1);
		bytes.push_back(std::byte(0x00));
	}
}

std::string charToCode(char c) {
	std::string code;
	std::string formatted = std::format("{:x}", (int)c);
	if ((int)c < 128) {
		code = "ASCII_" + Utility::String::toUpper(formatted);
	}
	else if ((int)c < 256) {
		code = "ISO_8859_1_" + Utility::String::toUpper(formatted);
	}
	else {
		code = "UNICODE_" + Utility::String::toUpper(formatted);
	}

	return code;
}

std::vector<std::byte> charToBytes(char c) {
	return codeToBytes(charToCode(c));
}

std::byte strInstrToByte(std::string instruction) {
	instruction = Utility::String::trim(instruction);
	instruction = Utility::String::toUpper(instruction);
	if (keyboardConfig.GetValue("KEY_" + instruction) != std::nullopt)
		return strToByte(keyboardConfig.GetValue("KEY_" + instruction).value());
	if (instruction == ("ESCAPE"))
		return strInstrToByte("ESC");
	if (instruction == ("DEL"))
		return strInstrToByte("DELETE");
	if (instruction == ("BREAK"))
		return strInstrToByte("PAUSE");
	if (instruction == ("CONTROL"))
		return strInstrToByte("CTRL");
	if (instruction == ("DOWNARROW"))
		return strInstrToByte("DOWN");
	if (instruction == ("UPARROW"))
		return strInstrToByte("UP");
	if (instruction == ("LEFTARROW"))
		return strInstrToByte("LEFT");
	if (instruction == ("RIGHTARROW"))
		return strInstrToByte("RIGHT");
	if (instruction == ("MENU"))
		return strInstrToByte("APP");
	if (instruction == ("WINDOWS"))
		return strInstrToByte("GUI");
	if (instruction == ("PLAY") || instruction == ("PAUSE"))
		return strInstrToByte("MEDIA_PLAY_PAUSE");
	if (instruction == ("STOP"))
		return strInstrToByte("MEDIA_STOP");
	if (instruction == ("MUTE"))
		return strInstrToByte("MEDIA_MUTE");
	if (instruction == ("VOLUMEUP"))
		return strInstrToByte("MEDIA_VOLUME_INC");
	if (instruction == ("VOLUMEDOWN"))
		return strInstrToByte("MEDIA_VOLUME_DEC");
	if (instruction == ("SCROLLLOCK"))
		return strInstrToByte("SCROLL_LOCK");
	if (instruction == ("NUMLOCK"))
		return strInstrToByte("NUM_LOCK");
	if (instruction == ("CAPSLOCK"))
		return strInstrToByte("CAPS_LOCK");
	return charToBytes(instruction[0])[0];
}

int main(int argc, char** argv) {
	if (argc != 2) { std::cout << "Filepath argument required!" << std::endl; return 0; }

	keyboardConfig = Configuration();
	layoutConfig = Configuration();
	std::cout << "Loading configuration files..." << std::endl;

	Configuration::LoadFromFile("resources/keyboard.cfg", keyboardConfig);
	std::cout << "|- Loaded 'keyboard.cfg'" << std::endl;
	Configuration::LoadFromFile("resources/gb.cfg", layoutConfig);
	std::cout << "|- Loaded 'gb.cfg'" << std::endl;

	std::cout << "Finished loading configuration." << std::endl;
	std::cout << std::endl;

	std::vector<std::string> lines;

	std::cout << "Loading .pscript file '" << argv[1] << "'" << std::endl;
	std::string line;
	std::fstream f(argv[1]);
	while (std::getline(f, line)) {
		line = Utility::String::trim(line);
		if (line == "") { continue; }
		if (line.starts_with("//")) { continue; }
		if (std::empty(line)) { continue; }
		lines.push_back(line);
	}
	std::cout << "Finished loading file." << std::endl;
	std::cout << std::endl;

	const int lineCount = lines.size();
	int prevP = 0;

	std::vector<std::byte> finalData;
	std::vector<std::byte> capturedBytes;
	std::vector<std::byte> prevBytes;
	std::vector<std::byte> currentBytes;
	bool group = false;
	
	const int defaultDelay = 0; // ms

	auto addByte = [&finalData, &capturedBytes, &group, &currentBytes](std::byte byte) {
		finalData.push_back(byte);
		currentBytes.push_back(byte);
		if (group)
			capturedBytes.push_back(byte);
	};

	std::cout << "Parsing file..." << std::endl;
	for (int l = 0; l < lineCount; l++) {
		int p = std::floor(((float)l / (float)lineCount)*10.f);
		if (prevP != p) {
			std::cout << "|- " << ((float)std::floor(((float)l / (float)lineCount) * 100.f)) << "%" << std::endl;
			prevP = p;
		}

		int delay = defaultDelay;

		auto splt = Utility::String::splitFirst(lines[l], " ");
		std::string instr = splt[0];
		std::string data;
		if (splt.size() == 2) {
			data = splt[1];
		}

		if (instr == "{") {
			capturedBytes.clear();
			group = true;
			continue;
		}
		else if (instr == "}") {
			group = false;
			continue;
		} else if (instr == "DELAY") {
			CRITICAL_ASSERT((data != ""), argv[1], l + 1, "Missing 'time' argument for keyword 'DELAY'.");
			CRITICAL_ASSERT((Utility::String::isInt(data)), argv[1], l + 1, "'time' argument for keyword 'DELAY' is not an integer.");
			delay = std::stoi(data);
		}
		else if (instr == "REPEAT") {
			CRITICAL_ASSERT((data != ""), argv[1], l + 1, "Missing 'amount' argument for keyword 'REPEAT'.");
			CRITICAL_ASSERT((l > 0), argv[1], l + 1, "No group or line above keyword 'REPEAT'.");
			bool useGroup = false;

			std::string prevLine = lines[l - 1];
			if (prevLine == "}") {
				useGroup = true;
			}
			
			for (int i = 0; i < std::stoi(data); i++) {
				addBytes(finalData, currentBytes, capturedBytes, false, useGroup ? capturedBytes : prevBytes);
			}
		}
		else if (instr == "STRING") {
			CRITICAL_ASSERT((data != ""), argv[1], l + 1, "Missing 'data' argument for keyword 'STRING'.");
			for (int i = 0; i < data.size(); i++) {
				addBytes(finalData, currentBytes, capturedBytes, group, charToBytes(data[i]));
			}
		}
		else if (instr == "CONTROL" || instr == "CTRL") {
			if (data != "") {
				addByte(strInstrToByte(data));
				addByte(strToByte(keyboardConfig.GetValue("MODIFIERKEY_CTRL").value()));
			}
			else {
				addByte(strToByte(keyboardConfig.GetValue("KEY_LEFT_CTRL").value()));
				addByte((std::byte)0x00);
			}
		}
		else if (instr == "ALT") {
			if (data != "") {
				addByte(strInstrToByte(data));
				addByte(strToByte(keyboardConfig.GetValue("MODIFIERKEY_ALT").value()));
			}
			else {
				addByte(strToByte(keyboardConfig.GetValue("KEY_LEFT_ALT").value()));
				addByte((std::byte)0x00);
			}
		}
		else if (instr == "SHIFT") {
			if (data != "") {
				addByte(strInstrToByte(data));
				addByte(strToByte(keyboardConfig.GetValue("MODIFIERKEY_SHIFT").value()));
			}
			else {
				addByte(strToByte(keyboardConfig.GetValue("KEY_LEFT_SHIFT").value()));
				addByte((std::byte)0x00);
			}
		}
		else if (instr == "CTRL-ALT") {
			if (data != "") {
				addByte(strInstrToByte(data));
				addByte(strToByte(keyboardConfig.GetValue("MODIFIERKEY_CTRL").value()) | strToByte(keyboardConfig.GetValue("MODIFIERKEY_ALT").value()));
			}
			else {
				continue;
			}
		}
		else if (instr == "CTRL-SHIFT") {
			if (data != "") {
				addByte(strInstrToByte(data));
				addByte(strToByte(keyboardConfig.GetValue("MODIFIERKEY_CTRL").value()) | strToByte(keyboardConfig.GetValue("MODIFIERKEY_SHIFT").value()));
			}
			else {
				continue;
			}
		}
		else if (instr == "COMMAND-OPTION") {
			if (data != "") {
				addByte(strInstrToByte(data));
				addByte(strToByte(keyboardConfig.GetValue("MODIFIERKEY_KEY_LEFT_GUI").value()) | strToByte(keyboardConfig.GetValue("MODIFIERKEY_ALT").value()));
			}
			else {
				continue;
			}
		}
		else if (instr == "ALT-SHIFT") {
			if (data != "") {
				addByte(strInstrToByte(data));
				addByte(strToByte(keyboardConfig.GetValue("MODIFIERKEY_LEFT_ALT").value()) | strToByte(keyboardConfig.GetValue("MODIFIERKEY_SHIFT").value()));
			}
			else {
				addByte(strToByte(keyboardConfig.GetValue("KEY_LEFT_ALT").value()));
				addByte(strToByte(keyboardConfig.GetValue("MODIFIERKEY_LEFT_ALT").value()) | strToByte(keyboardConfig.GetValue("MODIFIERKEY_SHIFT").value()));
			}
		}
		else if (instr == "ALT-TAB") {
			if (data == "") {
				addByte(strToByte(keyboardConfig.GetValue("KEY_TAB").value()));
				addByte(strToByte(keyboardConfig.GetValue("MODIFIERKEY_LEFT_ALT").value()));
			}
			else {
			}
		}
		else if (instr == "WINDOWS" || instr == "GUI") {
			if (data == "") {
				addByte(strToByte(keyboardConfig.GetValue("MODIFIERKEY_LEFT_GUI").value()));
				addByte((std::byte)0x00);
			}
			else {
				addByte(strInstrToByte(data));
				addByte(strToByte(keyboardConfig.GetValue("MODIFIERKEY_LEFT_GUI").value()));
			}
		}
		else if (instr == "COMMAND") {
			if (data == "") {
				addByte(strToByte(keyboardConfig.GetValue("KEY_COMMAND").value()));
				addByte((std::byte)0x00);
			}
			else {
				addByte(strInstrToByte(data));
				addByte(strToByte(keyboardConfig.GetValue("MODIFIERKEY_LEFT_GUI").value()));
			}
		}
		else {
			addByte(strInstrToByte(instr));
			addByte((std::byte)0x00);
		}


		// add delay
		while (delay > 0) {
			addByte((std::byte)0x00);
			if (delay > 255) {
				addByte((std::byte)0xFF);
				delay = delay - 255;
			}
			else {
				addByte((std::byte)delay);
				delay = 0;
			}
		}

		prevBytes = currentBytes;
		currentBytes.clear();
	}
	std::cout << "|- 100%" << std::endl;
	std::cout << "Finished parsing file." << std::endl;
	std::cout << std::endl;

	const int byteCount = finalData.size();
	int prevP2 = 0;

	std::cout << "Writing output file..." << std::endl;
	std::ofstream file(std::filesystem::path(argv[1]).filename().replace_extension("").string() + ".bin", std::ios::binary);

	char* d = new char[byteCount];
	for (int b = 0; b < byteCount; b++) {
		int p = std::floor(((float)b / (float)byteCount) * 10.f);
		if (prevP2 != p) {
			std::cout << "|- " << ((float)std::floor(((float)b / (float)byteCount) * 100.f)) << "%" << std::endl;
			prevP2 = p;
		}

		d[b] = (char)finalData[b];
	}
	file.write(d, byteCount);
	file.close();
	delete[] d;
	std::cout << "|- 100%" << std::endl;
	std::cout << "Finished writing to '" << std::filesystem::path(argv[1]).filename().replace_extension("").string() + ".bin'" << std::endl;
	std::cout << std::endl;

	std::cout << "Payload compiled." << std::endl;
}