#pragma once

#include "clay.h"
#include "clay_renderer_SDL2.h"
#include "collections.h"

static constexpr int CLAY_FONT_AMOUNT = 1;
static constexpr int CLAY_FONT_SIZE = 16;
static constexpr int FONT_ID_BODY_16 = 0;

typedef struct {
	SDL_Window *window;
	int windowWidth;
	int windowHeight;
	float yOffset;
	CardImagesVec *cardImages;
	SDL_Renderer *renderer;
	SDL2_Font *fonts;
} ClayData;

int
resize_rendering(void *userData, SDL_Event *event);
Clay_RenderCommandArray
mtg_card_search_create_layout(ClayData *userData);
void
handle_clay_errors(Clay_ErrorData errorData);