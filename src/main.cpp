#include "chip8/chip8.hpp"
#include <string>

int main()
{
	std::string filename = "MINIMALGAME.ch8";
	c_chip8 chip8{ filename };

	chip8.emulate();

	return 0;
}