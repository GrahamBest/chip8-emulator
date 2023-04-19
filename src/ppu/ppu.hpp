#pragma once

#include <SDL.h>
#include <string>
#include <cstdint>
#include <memory>

constexpr int WINDOW_WIDTH = 600;
constexpr int WINDOW_HEIGHT = 1200;

class c_ppu
{
public:
	c_ppu(const std::string& name)
	{
		if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
		{
			std::printf("FATAL ERROR SDL FAILED TO INITIALIZE\n");
		}


		SDL_Init(SDL_INIT_VIDEO);
		SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_WIDTH, 0, &this->window, &this->renderer); 
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		SDL_RenderSetScale(this->renderer, 10, 10);

		if (this->window == nullptr)
		{
			std::printf("FATAL ERROR WINDOW FAILED TO BE CREATED\n");
		}

		this->draw_surface = SDL_GetWindowSurface(this->get_window_ptr());
	}

	~c_ppu()
	{
		if (this->window != nullptr)
			SDL_DestroyWindow(this->window);

		if (this->renderer != nullptr)
			SDL_DestroyRenderer(this->renderer);
		SDL_Quit();
	}

	SDL_Window* get_window_ptr()
	{
		return this->window;
	}

	SDL_Surface* get_draw_surface()
	{
		return this->draw_surface;
	}
	
	SDL_Renderer* get_renderer()
	{
		return this->renderer;
	}
private:
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Surface* draw_surface;
};

static std::unique_ptr<c_ppu> ppu_ptr = std::make_unique<c_ppu>("Chip-8 Emulator by Graham");

namespace utility
{
	void set_pixel(std::uint32_t x, std::uint32_t y, std::uint8_t bit_n)
	{
		std::uint8_t* pixel = (std::uint8_t*)ppu_ptr->get_draw_surface()->pixels + y * ppu_ptr->get_draw_surface()->pitch + x * 4;
	}
}