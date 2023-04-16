#pragma once

#include <cstdint>
#include <iostream>
#include <concepts>

constexpr unsigned int MAX_REGISTERS = 21;

enum REGISTERS
{
	V0,
	V1,
	V2,
	V3,
	V4,
	V5,
	V6,
	V7,
	V8,
	V9,
	VA,
	VB,
	VC,
	VD,
	VE,
	VF,
	VI,
	V_DELAY,
	V_SOUND,
	VSP,
	PC
};

struct register_t
{
	union values
	{
		std::uint8_t value;
		std::uint16_t value16;

		register_t& operator++()
		{
			value++;
		}
	};

	values value_union;
};

class c_register
{
public:
	template<REGISTERS REG_ID, std::integral T>
	void set_value(T value)
	{
		if (std::is_same_v<T, std::uint16_t>)
		{
			this->register_array[REG_ID].value_union.value16 = value;
		}
		else if (std::is_same_v<T, std::uint8_t>)
		{
			this->register_array[REG_ID].value_union.value = value;
		}
	}

	template<enum REGISTERS REG_ID, std::integral T>
	void increment()
	{
		if (std::is_same_v<T, std::uint16_t>)
		{
			this->register_array[REG_ID].value_union.value16 += 1;
		}
		else if (std::is_same_v<T, std::uint8_t>)
		{
			this->register_array[REG_ID].value_union.value += 1;
		}
	}

	template<enum REGISTERS REG_ID, std::integral T>
	void decrement()
	{
		if (std::is_same_v<T, std::uint16_t>)
		{
			this->register_array[REG_ID].value_union.value16 -= 1;
		}
		else if (std::is_same_v<T, std::uint8_t>)
		{
			this->register_array[REG_ID].value_union.value -= 1;
		}
	}


	template<enum REGISTERS REG_ID, std::integral T>
	T get_value()
	{
		if (std::is_same_v<T, std::uint16_t>)
		{
			return this->register_array[REG_ID].value_union.value16;
		}
		else if (std::is_same_v<T, std::uint8_t>)
		{
			return this->register_array[REG_ID].value_union.value;
		}
	}

	register_t register_array[MAX_REGISTERS];
private:
};