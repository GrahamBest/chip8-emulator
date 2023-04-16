#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include "registers.hpp"

static std::unique_ptr<c_register> register_ptr = std::make_unique<c_register>();

class c_chip8
{
public:
	c_chip8(const std::string& filename);

	void emulate();

	std::unique_ptr<std::uint8_t[]> data{};
private:
	unsigned int length{};
	std::ifstream file;
};