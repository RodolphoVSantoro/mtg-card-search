#pragma once

#include "clay.h"
#include "clay_renderer_SDL2.h"
#include "collections.h"

#define FONT_ID_BODY_16 0

typedef struct {
	float yOffset;
	CardImagesVec *cardImages;
	int *windowWidth;
	int *windowHeight;
} ClayData;

struct ResizeRenderData_ {
	SDL_Window *window;
	int *windowWidth;
	int *windowHeight;
	ClayData demoData;
	SDL_Renderer *renderer;
	SDL2_Font *fonts;
};
typedef struct ResizeRenderData_ ResizeRenderData;

ClayData
CardSearchInitialize(CardImagesVec *cardImages, int *windowHeight,
					 int *windowWidth);
int
resizeRendering(void *userData, SDL_Event *event);
Clay_RenderCommandArray
ClayVideoDemo_CreateLayout(ClayData *userData);
void
HandleClayErrors(Clay_ErrorData errorData);