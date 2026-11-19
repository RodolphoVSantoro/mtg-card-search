#pragma once

#include "clay.h"
#include "sdl/SDL_ttf.h"

static constexpr double M_PI = 3.14159265358979323846;

#define CLAY_COLOR_TO_SDL_COLOR_ARGS(color) color.r, color.g, color.b, color.a

typedef struct {
	uint32_t fontId;
	TTF_Font *font;
} SDL2Font;

Clay_Dimensions
measure_text(Clay_StringSlice text, Clay_TextElementConfig *config,
			 void *userData);
void
clay_sdl2_render(SDL_Renderer *renderer, Clay_RenderCommandArray renderCommands,
				 SDL2Font *fonts);
