#pragma once

#include "chip8.hpp"
#include <random>
#include "../ppu/ppu.hpp"

enum HIOPCODE
{
	JP = 0x1000,
	CALL = 0x2000,
	SEVXBYTE = 0x3000,
	SNEVXBYTE = 0x4000,
	SEVXVY = 0x5000,
	LDVXBYTE = 0x6000,
	ADDVXBYTE = 0x7000,
	LD = 0x8000,
	SNEVXVY = 0x9000,
	LDIADDR = 0xA000,
	JPV0ADDR = 0xB000,
	RND = 0xC000,
	DRW = 0xD000,
	SKP = 0xE000,
	LDSPECIAL = 0xF000,

};

enum LOWOPCODE
{
	VXVY = 0x0,
	ORVXVY = 0x1,
	ANDVXVY = 0x2,
	XORVXVY = 0x3,
	ADDVXVY = 0x4,
	SUBVXVY = 0x5,
	SHRVX1 = 0x6,
	SUBNVXVY = 0x7,
	SHLVX1 = 0xE,

	SKPVX = 0x9E,
	SKNPVX = 0xA1,

	LDVXDT = 0x07,
	LDVXK = 0x0A,
	LDDTVX = 0x15,
	LDSTVX = 0x18,
	ADDIVX = 0x1E,
	LDFVX = 0x29,
	LDBVX = 0x33,
	LDIARRAYFROMV0VX = 0x55,
	LDV0VXFROMIARRAY = 0x65
};

namespace instructions
{
	/*
	*	CALL INSTRUCTION IMPLEMENTATION FOR CHIP8
	*/
	void call(std::uint16_t value)
	{
		register_ptr->increment<REGISTERS::VSP, std::uint8_t>();
		register_ptr->set_value<REGISTERS::VSP, std::uint8_t>(register_ptr->get_value<REGISTERS::PC, std::uint16_t>());
		register_ptr->set_value<REGISTERS::PC, std::uint16_t>(value - 0x200); // it's based at 0x200 and the roms are based at 0x00 so we need to fix the image
		// base by subtracting off 0x200
	}

	/*
	*	JP INSTRUCTION IMPLEMENTATION FOR CHIP8
	*/
	void jmp(std::uint16_t value)
	{
		register_ptr->set_value<REGISTERS::PC, std::uint16_t>(value - 0x200); // it's based at 0x200 and the roms are based at 0x00 so we need to fix the image
		// base by subtracting off 0x200
	}

	/*
	*	RETURN INSTRUCTION IMPLEMENTATION FOR CHIP8
	*/
	void ret()
	{
		register_ptr->set_value<REGISTERS::PC, std::uint16_t>(register_ptr->get_value<REGISTERS::VSP, std::uint8_t>());
		register_ptr->decrement<REGISTERS::VSP, std::uint8_t>();
	}
	/*
	*	SE INSTRUCTION IMPLEMENTATION FOR CHIP8
	*/
	void se(register_t& vx, std::uint8_t value)
	{
		if (vx.value_union.value == value)
		{
			register_ptr->register_array[REGISTERS::PC].value_union.value16 += 4;
		}
	}

	/*
	*	SNE INSTRUCTION IMPLEMENTATION FOR CHIP8
	*/
	void sne(const register_t& vx, std::uint8_t value)
	{
		if (vx.value_union.value != value)
		{
			register_ptr->register_array[REGISTERS::PC].value_union.value16 += 4;
		}
	}

	/*
	*	SE VX VY INSTRUCTION IMPLEMENTATION FOR CHIP8
	*/
	void se_registers(const register_t& vx, const register_t& vy)
	{
		if (vx.value_union.value == vy.value_union.value)
		{
			register_ptr->register_array[REGISTERS::PC].value_union.value16 += 4;
		}
	}

	/*
	*	LD INSTRUCTION IMPLEMENTATION FOR CHIP8
	*/
	void ld_byte(register_t& vx, std::uint8_t value)
	{
		vx.value_union.value = value;
	}

	/*
	*	ADD INSTRUCTION IMPLEMENTATION FOR CHIP8
	*/
	void add_byte(register_t& vx, std::uint8_t value)
	{
		vx.value_union.value += value;
	}

	/*
	*	LD VX VY INSTRUCTION IMPLEMENTATION FOR CHIP8
	*/
	void ld_registers(register_t& vx, const register_t& vy)
	{
		vx.value_union.value = vy.value_union.value;
	}

	/*
	*	OR VX VY INSTRUCTION IMPLEMENTATION FOR CHIP8
	*/
	void or_registers(register_t& vx, const register_t& vy)
	{
		vx.value_union.value |= vy.value_union.value;
	}

	/*
	*	AND VX VY INSTRUCTION IMPLEMENTATION FOR CHIP8
	*/
	void and_registers(register_t& vx, const register_t& vy)
	{
		vx.value_union.value &= vy.value_union.value;
	}

	/*
	*	XOR VX VY INSTRUCTION IMPLEMENTATION FOR CHIP8
	*/
	void xor_registers(register_t& vx, const register_t& vy)
	{
		vx.value_union.value ^= vy.value_union.value;
	}

	/*
	*	ADD VX VY INSTRUCTION IMPLEMENTATION FOR CHIP8
	*/
	void add_registers(register_t& vx, const register_t& vy)
	{
		std::uint16_t value = static_cast<std::uint16_t>(vx.value_union.value) + static_cast<std::uint16_t>(vy.value_union.value);

		if (value > 0xFF)
		{
			register_ptr->set_value<REGISTERS::VF, std::uint8_t>(1);
			vx.value_union.value = static_cast<std::uint8_t>(value);
		
			return;
		}

		vx.value_union.value = static_cast<std::uint8_t>(value);
	}

	/*
	*	SUB VX VY INSTRUCTION IMPLEMENTATION FOR CHIP8
	*/
	void sub_registers(register_t& vx, const register_t& vy)
	{
		std::uint8_t value = vx.value_union.value - vy.value_union.value;

		if (vx.value_union.value > vy.value_union.value)
		{
			register_ptr->set_value<REGISTERS::VF, std::uint8_t>(1);
			vx.value_union.value = value;

			return;
		}

		register_ptr->set_value<REGISTERS::VF, std::uint8_t>(0);
		vx.value_union.value = value;
	}

	/*
	*	SHR VX INSTRUCTION IMPLEMENTATION FOR CHIP8
	*/
	void shr(register_t& vx)
	{
		/* check if least significant bit is 1*/
		if (vx.value_union.value & 0x01)
		{
			register_ptr->set_value<REGISTERS::VF, std::uint8_t>(1);
			vx.value_union.value >>= 1;
			return;
		}

		register_ptr->set_value<REGISTERS::VF, std::uint8_t>(0);
		vx.value_union.value >>= 1;
	}

	/*
	*	SUBN VX VY INSTRUCTION IMPLEMENTATION FOR CHIP8
	*/
	void subn_registers(register_t& vx, const register_t& vy)
	{

		std::uint8_t value = vy.value_union.value - vx.value_union.value;

		if (vy.value_union.value > vx.value_union.value)
		{
			register_ptr->set_value<REGISTERS::VF, std::uint8_t>(1);
			vx.value_union.value = value;

			return;
		}

		register_ptr->set_value<REGISTERS::VF, std::uint8_t>(0);
		vx.value_union.value = value;
	}

	/*
	*	SHL VX INSTRUCTION IMPLEMENTATION FOR CHIP8
	*/
	void shl(register_t& vx)
	{
		/* check if most significant bit is 1 */
		if (vx.value_union.value & 0x80)
		{
			register_ptr->set_value<REGISTERS::VF, std::uint8_t>(1);
			vx.value_union.value <<= 1;
		
			return;
		}

		register_ptr->set_value<REGISTERS::VF, std::uint8_t>(0);
		vx.value_union.value <<= 1;
	}

	/*
	*	SNE VX VY INSTRUCTION IMPLEMENTATION FOR CHIP8
	*/
	void sne_register(const register_t& vx, const register_t& vy)
	{
		if (vx.value_union.value != vy.value_union.value)
		{
			register_ptr->register_array[REGISTERS::PC].value_union.value16 += 2;
		}
	}

	/*
	*	LD I, ADDR INSTRUCTION IMPLEMENTATION FOR CHIP8
	*/
	void ld_iaddr(std::uint16_t addr)
	{
		register_ptr->register_array[REGISTERS::VI].value_union.value16 = addr;
	}

	/*
	*	JP V0, ADDR INSTRUCTION IMPLEMENTATION FOR CHIP8
	*/
	void jmp_registerv0addr(std::uint16_t addr)
	{ 
		/* subtract 0x200 to rebase the image */
		register_ptr->set_value<REGISTERS::PC, std::uint16_t>((addr + static_cast<std::uint16_t>(register_ptr->register_array[REGISTERS::V0].value_union.value)) - 0x200);
	}

	/*
	*	RND VX, BYTE INSTRUCTION IMPLEMENTATION FOR CHIP8
	*/
	void rnd_registerbyte(register_t& vx, std::uint8_t byte)
	{
		static std::random_device rd;
		static std::mt19937 rng{ rd() };
		static std::uniform_int_distribution<std::uint32_t> uid(0, 0xFF);

		std::uint8_t value = uid(rng);

		vx.value_union.value = value & byte;
	}

	/* DRAW FUNCTION TO IMPLEMENT SOON */
	void draw(const register_t& vx, const register_t& vy, std::uint8_t n, std::uint8_t* data)
	{
		for (int i = 0; i < n; i++)
		{
			for (std::int8_t b = 7; b >= 0; b--)
			{
				for (int x = 0; x < n; x++)
				{
					std::uint8_t current_byte = data[register_ptr->register_array[REGISTERS::VI].value_union.value16 + x];

					std::uint8_t pixel = (current_byte >> b) & 1;

					if (pixel != 0)
						SDL_RenderDrawPoint(ppu_ptr->get_renderer(), vx.value_union.value + b * 3, vy.value_union.value + x * 3);
				}
			}
		}
	}

	/* SKIP IF PRESSED INSTRUCTION TO IMPLEMENT SOON*/

	/* SKIP IF NOT PRESSED TO IMPLEMENT SOON */

	/*
	*	LD VX, DT INSTRUCTION IMPLEMENTATION FOR CHIP8
	*/
	void ld_registerdt(register_t& vx)
	{
		vx.value_union.value = register_ptr->register_array[REGISTERS::V_DELAY].value_union.value;
	}

	/* LOAD KEY NUMBER INTO VX INSTRUCTION TO IMPLEMENT SOON */
	
	/*
	*	LD DT, VX INSTRUCTION IMPLEMENTATION FOR CHIP8
	*/
	void ld_registerintodt(const register_t& vx)
	{
		register_ptr->register_array[REGISTERS::V_DELAY].value_union.value = vx.value_union.value;
	}
	
	/*
	*	LD ST, VX INSTRUCTION IMPLEMENTATION FOR CHIP8
	*/
	void ld_registerintost(const register_t& vx)
	{
		register_ptr->register_array[REGISTERS::V_SOUND].value_union.value = vx.value_union.value;
	}

	/*
	*	ADD I, VX INSTRUCTION IMPLEMENTATION FOR CHIP8
	*/
	void add_ifromregister(register_t& i, const register_t& vy)
	{
		i.value_union.value16 += vy.value_union.value;
	}

	/* LD F, VX IMPLEMENTATION SOON */	

	/* LD B, VX IMPLEMENTATION SOON */

	/* LD [I], VX IMPLEMENTATION */
	void ld_iarrayfromregister(const std::uint8_t& n, std::uint8_t* data)
	{
		std::uint16_t original = register_ptr->get_value<REGISTERS::VI, std::uint16_t>();

		for (int i = 0; i <= n; i++)
		{
			REGISTERS reg = static_cast<REGISTERS>(i);
			data[register_ptr->register_array[REGISTERS::VI].value_union.value16] = register_ptr->register_array[reg].value_union.value; // it's based at 0x200 and the roms are based at 0x00 so we need to fix the image
			// base by subtracting off 0x200
			register_ptr->register_array[REGISTERS::VI].value_union.value16 += 1;
		}

		register_ptr->set_value<REGISTERS::VI, std::uint16_t>(original);
	}

	/*
	*	LD VX, [I] IMPLEMENTATION
	*/

	void ld_registerarrayi(const std::uint8_t& n, std::uint8_t* data)
	{
		std::uint16_t original = register_ptr->get_value<REGISTERS::VI, std::uint16_t>();
	
		for (int i = 0; i <= n; i++)
		{
			REGISTERS reg = static_cast<REGISTERS>(i);
			register_ptr->register_array[reg].value_union.value = data[register_ptr->register_array[REGISTERS::VI].value_union.value16];
			register_ptr->register_array[REGISTERS::VI].value_union.value16 += 1;
		}

		register_ptr->set_value<REGISTERS::VI, std::uint16_t>(original);
	}
}