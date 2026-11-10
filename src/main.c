#define CLAY_IMPLEMENTATION

#include "dependencies/clay.h"
#include "dependencies/clay_renderer_SDL2.c"
#include "sqlite3.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "dependencies/clay-video-demo.c"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void HandleClayErrors(Clay_ErrorData errorData) {
	printf("%s", errorData.errorText.chars);
}

struct ResizeRenderData_ {
	SDL_Window *window;
	int windowWidth;
	int windowHeight;
	ClayVideoDemo_Data demoData;
	SDL_Renderer *renderer;
	SDL2_Font *fonts;
};
typedef struct ResizeRenderData_ ResizeRenderData;

int resizeRendering(void *userData, SDL_Event *event) {
	ResizeRenderData *actualData = userData;
	if (event->type == SDL_WINDOWEVENT &&
		event->window.event == SDL_WINDOWEVENT_EXPOSED) {
		SDL_Window *window = actualData->window;
		int windowWidth = actualData->windowWidth;
		int windowHeight = actualData->windowHeight;
		ClayVideoDemo_Data demoData = actualData->demoData;
		SDL_Renderer *renderer = actualData->renderer;
		SDL2_Font *fonts = actualData->fonts;

		SDL_GetWindowSize(window, &windowWidth, &windowHeight);
		Clay_SetLayoutDimensions(
			(Clay_Dimensions){(float)windowWidth, (float)windowHeight});

		Clay_RenderCommandArray renderCommands =
			ClayVideoDemo_CreateLayout(&demoData);
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);

		Clay_SDL2_Render(renderer, renderCommands, fonts);

		SDL_RenderPresent(renderer);
	}
	return 0;
}

int main(int argc, char *argv[]) {
	bool sdlInitiated = false, ttfInitiated = false, imgInitiated = false;
	int returnVal = 0;
	sqlite3 *imageDb = NULL, *cardDb = NULL;
	int error = sqlite3_open("resources/scryfall_images.db", &imageDb);
	if (error != SQLITE_OK) {
		fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(imageDb));
		returnVal = 1;
		goto quit;
	}
	error = sqlite3_open("resouces/AllPrintings.sqlite", &cardDb);
	if (error != SQLITE_OK) {
		fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(cardDb));
		returnVal = 1;
		goto quit;
	}

	// SDL + Clay testing
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "Error: could not initialize SDL: %s\n",
				SDL_GetError());
		returnVal = 1;
		goto quit;
	}
	sdlInitiated = true;
	if (TTF_Init() < 0) {
		fprintf(stderr, "Error: could not initialize TTF: %s\n",
				TTF_GetError());
		returnVal = 1;
		goto quit;
	}
	ttfInitiated = true;
	if (IMG_Init(IMG_INIT_PNG) < 0) {
		fprintf(stderr, "Error: could not initialize IMG: %s\n",
				IMG_GetError());
		returnVal = 1;
		goto quit;
	}
	imgInitiated = true;

	TTF_Font *robotoFont = TTF_OpenFont("resources/Roboto-Regular.ttf", 16);
	if (!robotoFont) {
		fprintf(stderr, "Error: could not load font: %s\n", TTF_GetError());
		returnVal = 1;
		goto quit;
	}

	SDL2_Font fonts[1] = {};

	fonts[FONT_ID_BODY_16] = (SDL2_Font){
		.fontId = FONT_ID_BODY_16,
		.font = robotoFont,
	};

	// SDL_Surface *sample_image = CreateSurfaceFromAVIF(blob_data_sdl, blob_size_sdl);

	SDL_Window *window = NULL;
	SDL_Renderer *renderer = NULL;

	SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl"); // for antialiasing
	window = SDL_CreateWindow("SDL", SDL_WINDOWPOS_UNDEFINED,
							  SDL_WINDOWPOS_UNDEFINED, 800, 600,
							  SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4); // for antialiasing

	bool enableVsync = false;
	if (enableVsync) {
		renderer = SDL_CreateRenderer(
			window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	}
	else {
		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	}
	SDL_SetRenderDrawBlendMode(renderer,
							   SDL_BLENDMODE_BLEND); // for alpha blending

	uint64_t totalMemorySize = Clay_MinMemorySize();
	Clay_Arena clayMemory = Clay_CreateArenaWithCapacityAndMemory(
		totalMemorySize, malloc(totalMemorySize));

	int windowWidth = 0;
	int windowHeight = 0;
	SDL_GetWindowSize(window, &windowWidth, &windowHeight);
	Clay_Initialize(clayMemory,
					(Clay_Dimensions){(float)windowWidth, (float)windowHeight},
					(Clay_ErrorHandler){HandleClayErrors, NULL});

	Clay_SetMeasureTextFunction(SDL2_MeasureText, &fonts);

	// Uint64 NOW = SDL_GetPerformanceCounter();
	// Uint64 LAST = 0;
	double deltaTime = 0;
	ClayVideoDemo_Data demoData = ClayVideoDemo_Initialize(sample_image);

	ResizeRenderData userData = {
		window,		  // SDL_Window*
		windowWidth,  // int
		windowHeight, // int
		demoData,	  // CustomShit
		renderer,	  // SDL_Renderer*
		fonts,		  // SDL2_Font[1]
	};
	// add an event watcher that will render the screen while youre dragging the
	// window to different sizes
	SDL_AddEventWatch(resizeRendering, &userData);

	while (true) {
		Clay_Vector2 scrollDelta = {};
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT: {
				goto quit;
			}
			case SDL_MOUSEWHEEL: {
				scrollDelta.x = event.wheel.x * 10;
				scrollDelta.y = event.wheel.y * 10;
				break;
			}
			}
		}

		int mouseX = 0;
		int mouseY = 0;
		Uint32 mouseState = SDL_GetMouseState(&mouseX, &mouseY);
		Clay_Vector2 mousePosition =
			(Clay_Vector2){(float)mouseX, (float)mouseY};
		Clay_SetPointerState(mousePosition, mouseState & SDL_BUTTON(1));

		Clay_UpdateScrollContainers(
			true, (Clay_Vector2){scrollDelta.x, scrollDelta.y}, deltaTime);

		SDL_GetWindowSize(window, &windowWidth, &windowHeight);
		Clay_SetLayoutDimensions(
			(Clay_Dimensions){(float)windowWidth, (float)windowHeight});

		Clay_RenderCommandArray renderCommands =
			ClayVideoDemo_CreateLayout(&demoData);
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);

		Clay_SDL2_Render(renderer, renderCommands, fonts);

		SDL_RenderPresent(renderer);
	}

quit:
	if(blob_data_sdl != NULL) {
		free(blob_data_sdl);
		blob_data_sdl = NULL;
	}
	if(imageDb != NULL) {
		sqlite3_close(imageDb);
		imageDb = NULL;
	}
	if(cardDb != NULL) {
		sqlite3_close(cardDb);
		cardDb = NULL;
	}
	if(renderer != NULL) {
		SDL_DestroyRenderer(renderer);
		renderer = NULL;
	}
	if(window != NULL) {
		SDL_DestroyWindow(window);
		window = NULL;
	}
	if (robotoFont != NULL) {
	    TTF_CloseFont(robotoFont);
	    robotoFont = NULL;
	}
	IMG_Quit();
	TTF_Quit();
	SDL_Quit();
	return returnVal;
}
