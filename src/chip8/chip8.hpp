#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <vector>
#include <bitset>
#include "registers.hpp"

static std::unique_ptr<c_register> register_ptr = std::make_unique<c_register>();

constexpr int MAX_FONTSET_BYTES = 0x50;

class c_chip8
{
public:
	c_chip8(const std::string& filename);

	void emulate();
	void setup_fontset();

	std::unique_ptr<std::uint8_t[]> data{};
	std::uint8_t* ptr_to_fontset{};
private:
	unsigned int length{};
	std::ifstream file;
};