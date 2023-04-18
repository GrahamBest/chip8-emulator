#include "chip8.hpp"
#include "instructions.hpp"
#include "../ppu/ppu.hpp"
#include <memory>

c_chip8::c_chip8(const std::string& filename)
{
	this->file.open(filename, std::ios::binary | std::ios::in);

	if (this->file.is_open())
	{
		this->file.seekg(0, std::ios::end);
		this->length = this->file.tellg();
		/* we add the max fontset bytes so we can put the data there ourselves. */
		this->data = std::make_unique<std::uint8_t[]>(this->length + MAX_FONTSET_BYTES);
		this->file.seekg(0, std::ios::beg);
		this->file.read(reinterpret_cast<char*>(this->data.get()), this->length);

		this->setup_fontset();
	}
	else
	{
		std::printf("EMULATOR FATAL ERROR: Couldn't open %s for writing!\n", filename.c_str());
	}
}

void c_chip8::setup_fontset()
{

	std::uint8_t* ptr = &this->data[this->length];

	std::uint8_t fontset[MAX_FONTSET_BYTES] =
	{
		0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
		0x20, 0x60, 0x20, 0x20, 0x70, // 1
		0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
		0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
		0x90, 0x90, 0xF0, 0x10, 0x10, // 4
		0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
		0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
		0xF0, 0x10, 0x20, 0x40, 0x40, // 7
		0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
		0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
		0xF0, 0x90, 0xF0, 0x90, 0x90, // A
		0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
		0xF0, 0x80, 0x80, 0x80, 0xF0, // C
		0xE0, 0x90, 0x90, 0x90, 0xE0, // D
		0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
		0xF0, 0x80, 0xF0, 0x80, 0x80  // F
	};

	for (int i = 0; i < MAX_FONTSET_BYTES; i++)
	{
		ptr[i] = fontset[i];
	}
};

void c_chip8::emulate()
{
	SDL_Event evnt;

	while (true)
	{
		std::uint16_t high_bits = this->data[register_ptr->register_array[REGISTERS::PC].value_union.value16];
		std::uint8_t low_bits = this->data[register_ptr->register_array[REGISTERS::PC].value_union.value16 + 1];
		std::uint16_t opcode = static_cast<std::uint16_t>(high_bits);
		opcode <<= 8;
		opcode |= low_bits;

		std::printf("%X%X \n", high_bits, low_bits);

		std::uint16_t opcode_instruction = opcode & 0xF000;

		switch (opcode_instruction)
		{
			case HIOPCODE::JP:
			{
				std::uint16_t value = opcode & 0x0FFF;
				value -= 0x200; // restore to new 0x00 base. .code sections in rom images are based at 0x200;
				instructions::jmp(value);
				break;
			}

			case HIOPCODE::CALL:
			{
				std::uint16_t value = opcode & 0x0FFF;
				instructions::call(value);
				break;
			}

			case HIOPCODE::SEVXBYTE:
			{
				std::uint16_t reg_id = opcode & 0x0F00;
				reg_id >>= 8;

				std::uint8_t value = static_cast<std::uint8_t>(opcode & 0x00FF);

				register_t& reg = reinterpret_cast<register_t&>(register_ptr->register_array[reg_id]);

				instructions::se(reg, value);
				break;
			}

			case HIOPCODE::SNEVXBYTE:
			{
				std::uint16_t reg_id = opcode & 0x0F00;
				reg_id >>= 8;
				
				std::uint8_t value = static_cast<std::uint8_t>(opcode & 0x00FF);
				
				register_t& reg = reinterpret_cast<register_t&>(register_ptr->register_array[reg_id]);
				instructions::sne(reg, value);
				break;
			}

			case HIOPCODE::SEVXVY:
			{
				std::uint16_t regx_id = opcode & 0x0F00;
				regx_id >>= 8;

				std::uint16_t regy_id = opcode & 0x00F0;
				regy_id >>= 4;

				register_t& x = reinterpret_cast<register_t&>(register_ptr->register_array[regx_id]);
				register_t& y = reinterpret_cast<register_t&>(register_ptr->register_array[regy_id]);

				instructions::se_registers(x, y);
				break;
			}

			case HIOPCODE::LDVXBYTE:
			{
				std::uint16_t reg_id = opcode & 0x0F00;
				reg_id >>= 8;
				std::uint8_t value = static_cast<std::uint8_t>(opcode & 0x00FF);
				register_t& reg = reinterpret_cast<register_t&>(register_ptr->register_array[reg_id]);

				instructions::ld_byte(reg, value);

				break;
			}

			case HIOPCODE::LD:
			{
				std::uint8_t lower_4bit_code = opcode & 0x000F;
				
				switch (lower_4bit_code)
				{
					case LOWOPCODE::VXVY:
					{
						std::uint16_t regx_id = opcode & 0x0F00;
						regx_id >>= 8;

						std::uint16_t regy_id = opcode & 0x00F0;
						regy_id >>= 4;


						register_t& regx = reinterpret_cast<register_t&>(register_ptr->register_array[regx_id]);
						register_t& regy = reinterpret_cast<register_t&>(register_ptr->register_array[regy_id]);

						instructions::ld_registers(regx, regy);
						break;
					}

					case LOWOPCODE::ORVXVY:
					{
						std::uint16_t regx_id = opcode & 0x0F00;
						regx_id >>= 8;

						std::uint16_t regy_id = opcode & 0x00F0;
						regy_id >>= 4;


						register_t& regx = reinterpret_cast<register_t&>(register_ptr->register_array[regx_id]);
						register_t& regy = reinterpret_cast<register_t&>(register_ptr->register_array[regy_id]);

						instructions::or_registers(regx, regy);
						break;
					}


					case LOWOPCODE::ANDVXVY:
					{
						std::uint16_t regx_id = opcode & 0x0F00;
						regx_id >>= 8;

						std::uint16_t regy_id = opcode & 0x00F0;
						regy_id >>= 4;


						register_t& regx = reinterpret_cast<register_t&>(register_ptr->register_array[regx_id]);
						register_t& regy = reinterpret_cast<register_t&>(register_ptr->register_array[regy_id]);

						instructions::and_registers(regx, regy);
						break;
					}

					case LOWOPCODE::XORVXVY:
					{
						std::uint16_t regx_id = opcode & 0x0F00;
						regx_id >>= 8;

						std::uint16_t regy_id = opcode & 0x00F0;
						regy_id >>= 4;


						register_t& regx = reinterpret_cast<register_t&>(register_ptr->register_array[regx_id]);
						register_t& regy = reinterpret_cast<register_t&>(register_ptr->register_array[regy_id]);

						instructions::xor_registers(regx, regy);
						break;
					}

					case LOWOPCODE::ADDVXVY:
					{
						std::uint16_t regx_id = opcode & 0x0F00;
						regx_id >>= 8;

						std::uint16_t regy_id = opcode & 0x00F0;
						regy_id >>= 4;


						register_t& regx = reinterpret_cast<register_t&>(register_ptr->register_array[regx_id]);
						register_t& regy = reinterpret_cast<register_t&>(register_ptr->register_array[regy_id]);

						instructions::add_registers(regx, regy);
						break;
					}

					case LOWOPCODE::SUBVXVY:
					{
						std::uint16_t regx_id = opcode & 0x0F00;
						regx_id >>= 8;

						std::uint16_t regy_id = opcode & 0x00F0;
						regy_id >>= 4;


						register_t& regx = reinterpret_cast<register_t&>(register_ptr->register_array[regx_id]);
						register_t& regy = reinterpret_cast<register_t&>(register_ptr->register_array[regy_id]);

						instructions::sub_registers(regx, regy);
						break;
					}

					case LOWOPCODE::SHRVX1:
					{
						std::uint16_t reg_id = opcode & 0x0F00;
						reg_id >>= 8;

						register_t& reg = reinterpret_cast<register_t&>(register_ptr->register_array[reg_id]);

						instructions::shr(reg);
						break;
					}

					case LOWOPCODE::SUBNVXVY:
					{
						std::uint16_t regx_id = opcode & 0x0F00;
						regx_id >>= 8;

						std::uint16_t regy_id = opcode & 0x00F0;
						regy_id >>= 4;


						register_t& regx = reinterpret_cast<register_t&>(register_ptr->register_array[regx_id]);
						register_t& regy = reinterpret_cast<register_t&>(register_ptr->register_array[regy_id]);

						instructions::subn_registers(regx, regy);
						break;
					}


					case LOWOPCODE::SHLVX1:
					{
						std::uint16_t reg_id = opcode & 0x0F00;
						reg_id >>= 8;

						register_t& reg = reinterpret_cast<register_t&>(register_ptr->register_array[reg_id]);

						instructions::shl(reg);
						break;
					}
				}

				break;
			}

			case HIOPCODE::SNEVXVY:
			{
				std::uint16_t regx_id = opcode & 0x0F00;
				regx_id >>= 8;

				std::uint16_t regy_id = opcode & 0x00F0;
				regy_id >>= 4;


				register_t& regx = reinterpret_cast<register_t&>(register_ptr->register_array[regx_id]);
				register_t& regy = reinterpret_cast<register_t&>(register_ptr->register_array[regy_id]);

				instructions::sne_register(regx, regy);
				break;
			}

			case HIOPCODE::LDIADDR:
			{
				std::uint16_t value = opcode & 0x0FFF;
				value -= 0x200; /* rebase the image by subtracting 0x200 */
				instructions::ld_iaddr(value);
				break;
			}

			case HIOPCODE::JPV0ADDR:
			{
				std::uint16_t value = opcode & 0x0FFF;

				instructions::jmp_registerv0addr(value);
				break;
			}

			case HIOPCODE::RND:
			{
				std::uint16_t reg_id = opcode & 0x0F00;
				reg_id >>= 8;
				register_t& reg = reinterpret_cast<register_t&>(register_ptr->register_array[reg_id]);
			
				std::uint8_t value = static_cast<std::uint8_t>(opcode & 0x00FF);

				instructions::rnd_registerbyte(reg, value);
				break;
			}

			case HIOPCODE::DRW:
			{
				std::uint16_t regx_id = opcode & 0x0F00;
				regx_id >>= 8;
				register_t& regx = reinterpret_cast<register_t&>(register_ptr->register_array[regx_id]);

				std::uint16_t regy_id = opcode & 0x00F0;
				regy_id >>= 4;
				register_t& regy = reinterpret_cast<register_t&>(register_ptr->register_array[regy_id]);

				std::uint8_t n = opcode & 0x000F;

				instructions::draw(regx, regx, n, this->data.get());
				break;
			}

			/* implement skp if pressed or not pressed here soon ! */
			case HIOPCODE::SKP:
			{
				std::uint8_t lower_byte_code = static_cast<std::uint8_t>(opcode & 0x00FF);
				
				switch (lower_byte_code)
				{
					case LOWOPCODE::SKPVX:
					{
						std::uint16_t regx_id = opcode & 0x0F00;
						regx_id >>= 8;
						register_t& regx = reinterpret_cast<register_t&>(register_ptr->register_array[regx_id]);

						instructions::skip_if_pressed(regx, evnt);
						
						break;
					}

					case LOWOPCODE::SKNPVX:
					{
						std::uint16_t regx_id = opcode & 0x0F00;
						regx_id >>= 8;
						register_t& regx = reinterpret_cast<register_t&>(register_ptr->register_array[regx_id]);

						instructions::skip_if_not_pressed(regx, evnt);

						break;
					}
				}
			
				break;
			}

			case HIOPCODE::LDSPECIAL:
			{
				std::uint8_t lower_byte_code = static_cast<std::uint8_t>(opcode & 0x00FF);

				switch (lower_byte_code)
				{
					case LOWOPCODE::LDVXDT:
					{	
						std::uint16_t reg_id = opcode & 0x0F00;
						reg_id >>= 8;

						register_t& reg = reinterpret_cast<register_t&>(register_ptr->register_array[reg_id]);

						instructions::ld_registerdt(reg);
						break;
					}
					case LOWOPCODE::LDVXK:
					{	std::uint16_t reg_id = opcode & 0x0F00;
						reg_id >>= 8;

						register_t& reg = reinterpret_cast<register_t&>(register_ptr->register_array[reg_id]);

						instructions::ld_key_into_register(reg, evnt);
						break;
					}
					case LOWOPCODE::LDDTVX:
					{
						std::uint16_t reg_id = opcode & 0x0F00;
						reg_id >>= 8;

						register_t& reg = reinterpret_cast<register_t&>(register_ptr->register_array[reg_id]);

						instructions::ld_registerintodt(reg);
						break;
					}
					case LOWOPCODE::LDSTVX:
					{
						std::uint16_t reg_id = opcode & 0x0F00;
						reg_id >>= 8;

						register_t& reg = reinterpret_cast<register_t&>(register_ptr->register_array[reg_id]);

						instructions::ld_registerintost(reg);
						break;
					}
					case LOWOPCODE::ADDIVX:
					{
						std::uint16_t reg_id = opcode & 0x0F00;
						reg_id >>= 8;

						register_t& reg = reinterpret_cast<register_t&>(register_ptr->register_array[reg_id]);
						register_t& i = reinterpret_cast<register_t&>(register_ptr->register_array[REGISTERS::VI]);

						instructions::add_ifromregister(i, reg);
						break;
					}
					case LOWOPCODE::LDFVX:
					{
						/*IMPLEMENTATION FOR LD F, VX*/
						std::uint16_t reg_id = opcode & 0x0F00;
						reg_id >>= 8;

						register_t& reg = reinterpret_cast<register_t&>(register_ptr->register_array[reg_id]);

						instructions::ld_fvx(reg, this->length);
						break;
					}
					case LOWOPCODE::LDBVX:
					{
						std::uint16_t reg_id = opcode & 0x0F00;
						reg_id >>= 8;

						register_t& reg = reinterpret_cast<register_t&>(register_ptr->register_array[reg_id]);

						instructions::ld_bvx(reg, this->data.get());
						break;
					}
					case LOWOPCODE::LDIARRAYFROMV0VX:
					{
						std::uint16_t byte = opcode & 0x0F00;
						byte >>= 8;

						std::uint8_t n = static_cast<std::uint8_t>(byte);


						instructions::ld_iarrayfromregister(n, this->data.get());

						break;
					}
					case LOWOPCODE::LDV0VXFROMIARRAY:
					{
						std::uint16_t byte = opcode & 0x0F00;
						byte >>= 8;

						std::uint8_t n = static_cast<std::uint8_t>(byte);

						instructions::ld_registerarrayi(n, this->data.get());
						break;
					}

					break;
				}

				break;
			}
		}

		if (SDL_PollEvent(&evnt) && evnt.type == SDL_QUIT)
			break;

		SDL_RenderPresent(ppu_ptr->get_renderer());
		SDL_SetRenderDrawColor(ppu_ptr->get_renderer(), 0, 0, 0, 0);
		SDL_RenderClear(ppu_ptr->get_renderer());
		register_ptr->register_array[REGISTERS::PC].value_union.value16 += 2;
	}
}