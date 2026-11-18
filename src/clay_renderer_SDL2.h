#pragma once

#include "clay.h"
#include "sdl/SDL_ttf.h"

#ifndef M_PI
#define M_PI 3.14159
#endif

#define CLAY_COLOR_TO_SDL_COLOR_ARGS(color) color.r, color.g, color.b, color.a

typedef struct {
	uint32_t fontId;
	TTF_Font *font;
} SDL2_Font;

Clay_Dimensions
SDL2_MeasureText(Clay_StringSlice text, Clay_TextElementConfig *config,
				 void *userData);
void
Clay_SDL2_Render(SDL_Renderer *renderer, Clay_RenderCommandArray renderCommands,
				 SDL2_Font *fonts);