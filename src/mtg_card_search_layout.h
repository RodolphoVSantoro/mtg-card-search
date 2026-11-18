#pragma once

#include "clay.h"
#include "clay_renderer_SDL2.h"
#include "collections.h"

#define FONT_ID_BODY_16 0

typedef struct {
	Clay_String title;
	Clay_String contents;
} Document;

typedef struct {
	Document *documents;
	uint32_t length;
} DocumentArray;

typedef struct {
	intptr_t offset;
	intptr_t memory;
} ClayVideoDemo_Arena;

typedef struct {
	float yOffset;
	ClayVideoDemo_Arena frameArena;
	CardImagesVec *cardImages;
	int *windowWidth;
	int *windowHeight;
} ClayVideoDemo_Data;

typedef struct {
	int32_t requestedDocumentIndex;
	int32_t *selectedDocumentIndex;
} SidebarClickData;

struct ResizeRenderData_ {
	SDL_Window *window;
	int *windowWidth;
	int *windowHeight;
	ClayVideoDemo_Data demoData;
	SDL_Renderer *renderer;
	SDL2_Font *fonts;
};
typedef struct ResizeRenderData_ ResizeRenderData;

ClayVideoDemo_Data ClayVideoDemo_Initialize(CardImagesVec *cardImages,
											int *windowHeight,
											int *windowWidth);
int resizeRendering(void *userData, SDL_Event *event);
Clay_RenderCommandArray
ClayVideoDemo_CreateLayout(ClayVideoDemo_Data *userData);
void HandleClayErrors(Clay_ErrorData errorData);