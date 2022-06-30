#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <functional>
#include <chrono>
#include <thread>
#include <cstdint>

#include <Windows.h>
#include <WinUser.h>


#include "utils/conversions.h"

void InputUpdate(WORD wVk, bool release) {
	INPUT input[1] = { 0 };
	input[0].type = INPUT_KEYBOARD;
	input[0].ki.wVk = wVk;
	input[0].ki.wScan = MapVirtualKeyEx(wVk, 0, (HKL)0x00000809);
	input[0].ki.dwFlags = (release ? KEYEVENTF_KEYUP : 0);

	UINT ret = SendInput(1, input, sizeof(INPUT));
}

int main(int argc, char** argv) 
{
    if (argc != 2) { std::cout << "Filepath argument required!" << std::endl; std::cin.get(); return 0; }
	std::cout << "Running..." << std::endl;

	std::vector<std::byte> buffer;

	std::ifstream in(argv[1], std::ios::binary);
	char c;
	while (in.get(c))
	{
		buffer.push_back((std::byte)c);
	}

	for (uint64_t cursor = 0; cursor < buffer.size(); cursor+=2) {
		if (buffer[cursor] == std::byte(0x00)) {
			if (std::to_integer<int>(buffer[cursor + 1]) == 0) {
				continue;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(std::to_integer<int>(buffer[cursor+1])));
		}
		else {
			WORD keywVk = MapVirtualKeyEx(Utility::Conversions::usbUsageIndexToScanCode.find(std::to_integer<uint8_t>(buffer[cursor]))->second, MAPVK_VSC_TO_VK_EX, LoadKeyboardLayoutA("00000809", 0));
			if (std::to_integer<uint8_t>(buffer[cursor + 1]) != 0) {
				WORD modifierKeywVk = MapVirtualKeyEx(Utility::Conversions::usbUsageIndexToScanCode.find(Utility::Conversions::modifierMaskToUSBUsageIndex.find(std::to_integer<uint8_t>(buffer[cursor + 1]))->second)->second, MAPVK_VSC_TO_VK_EX, LoadKeyboardLayoutA("00000809", 0));
				InputUpdate(modifierKeywVk, false);
				InputUpdate(keywVk, false);
				InputUpdate(keywVk, true);
				InputUpdate(modifierKeywVk, true);
			}
			else {
				InputUpdate(keywVk, false);
				InputUpdate(keywVk, true);
			}
		}
	}

	std::cout << "Finished." << std::endl;
}