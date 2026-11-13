#include "collections.h"
#include "mtg_imgs.h"
#include <SDL2/SDL_surface.h>
#define CLAY_IMPLEMENTATION

#include "clay_renderer_SDL2.c"
#include "dependencies/clay.h"
#include "sqlite3.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "clay-video-demo.c"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define TARGET_FPS 240
#define FRAME_TARGET_TIME (1000 / TARGET_FPS)

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
	CardImagesVec cardImages = card_images_vec_init();
	CStringVec strs = cstring_vec_init();
	bool sdlInitiated = false, ttfInitiated = false, imgInitiated = false;
	int returnVal = 0;
	sqlite3 *imageDb = NULL, *cardDb = NULL;

	int error = sqlite3_open("./resources/all_cards.db", &cardDb);
	if (error != SQLITE_OK) {
		fprintf(stderr, "Cannot open card database: %s(%d)\n",
				sqlite3_errmsg(cardDb), error);
		returnVal = 1;
		goto quit;
	}

	error = sqlite3_open("./resources/scryfall_images.db", &imageDb);
	if (error != SQLITE_OK) {
		fprintf(stderr, "Cannot open image database: %s(%d)\n",
				sqlite3_errmsg(imageDb), error);
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

	SDL_Window *window = NULL;
	SDL_Renderer *renderer = NULL;

	SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl"); // for antialiasing
	window = SDL_CreateWindow("SDL", SDL_WINDOWPOS_UNDEFINED,
							  SDL_WINDOWPOS_UNDEFINED, 800, 600,
							  SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4); // for antialiasing

	bool enableVsync = true;
	if (enableVsync) {
		renderer = SDL_CreateRenderer(
			window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	} else {
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

	cstring_vec_push(&strs,
					 cstring_from_str("f935adbb-d2e3-48d0-b38c-41f3b583fecb"));
	cstring_vec_push(&strs,
					 cstring_from_str("8d887f3d-f07f-4672-a3a2-acdc42fa98e2"));
	cstring_vec_push(&strs,
					 cstring_from_str("0817c5fa-acd9-4fed-ab8b-06750a8817b0"));
	cstring_vec_push(&strs,
					 cstring_from_str("6dccd622-264e-4867-b34b-324c94861241"));
	cstring_vec_push(&strs,
					 cstring_from_str("b63d828d-4f62-424b-969a-00dcc4651b56"));
	cstring_vec_push(&strs,
					 cstring_from_str("97382dd8-2754-4ca3-8ba8-d655acaf22ac"));
	cstring_vec_push(&strs,
					 cstring_from_str("075947cd-2e06-486a-83a7-97321445a679"));
	cstring_vec_push(&strs,
					 cstring_from_str("f248d8a1-7148-4dbb-b1b3-edb476666fef"));
	cstring_vec_push(&strs,
					 cstring_from_str("806d854b-a165-4d40-86d7-d6a0aa17ce18"));
	cstring_vec_push(&strs,
					 cstring_from_str("dbcdf625-d257-48b8-8714-f49d99135706"));
	cstring_vec_push(&strs,
					 cstring_from_str("95089b00-b159-4c95-952d-bc29e463a1e7"));
	cstring_vec_push(&strs,
					 cstring_from_str("cfabc340-0391-4019-8e96-f010177b1d68"));
	cstring_vec_push(&strs,
					 cstring_from_str("faa526aa-f569-486a-8712-47c972481cd4"));
	cstring_vec_push(&strs,
					 cstring_from_str("889a828d-8915-4c2b-8650-f368172890e8"));
	cstring_vec_push(&strs,
					 cstring_from_str("2908c551-6798-4f05-826b-714b4ef131f7"));
	cstring_vec_push(&strs,
					 cstring_from_str("56808e49-1926-4f3f-93d6-a37213c760b2"));
	cstring_vec_push(&strs,
					 cstring_from_str("38f5512b-e7c4-43cd-9bfd-e95681a5b050"));
	cstring_vec_push(&strs,
					 cstring_from_str("761172c9-a215-4bc1-85de-cca36ba0f5c2"));
	cstring_vec_push(&strs,
					 cstring_from_str("1f68c4c2-91b5-4ffe-9dff-a6834038aa94"));
	cstring_vec_push(&strs,
					 cstring_from_str("bf55e49b-4186-4d36-b1f0-4c7715be0f63"));
	cstring_vec_push(&strs,
					 cstring_from_str("6d4c853e-2231-4af2-bcb0-1781c18ec3be"));
	cstring_vec_push(&strs,
					 cstring_from_str("06013d41-9042-46ed-985a-6e0d3b2890b8"));
	cstring_vec_push(&strs,
					 cstring_from_str("62209251-4118-4843-895b-46afb7284c75"));
	cstring_vec_push(&strs,
					 cstring_from_str("e3b425cd-c5a5-48e9-b697-3860dfa6d5d3"));
	cstring_vec_push(&strs,
					 cstring_from_str("4b9170f0-d332-42ba-98c7-7e99922fa3fb"));
	cstring_vec_push(&strs,
					 cstring_from_str("2e7f95d5-b279-4469-9c89-1e02630d61e6"));
	cstring_vec_push(&strs,
					 cstring_from_str("f72ed0e5-37a0-4909-a37e-ba4745bfae3b"));
	cstring_vec_push(&strs,
					 cstring_from_str("22009c47-3b0e-41f9-843d-f670d8afda23"));
	cstring_vec_push(&strs,
					 cstring_from_str("eb9dbad5-7df6-4f78-b809-d6043bf15d4b"));
	// cstring_vec_push(&strs,
	// cstring_from_str("499ae191-80ac-42bf-b49d-db343803bd56"));

	get_card_blobs_by_id(renderer, imageDb, &cardImages, strs);
	ClayVideoDemo_Data demoData = ClayVideoDemo_Initialize(&cardImages);

	cstring_vec_free(&strs);

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

	Uint64 NOW = SDL_GetPerformanceCounter();
	while (true) {
		Uint64 LAST = NOW;
		Uint64 NOW = SDL_GetPerformanceCounter();
		double deltaTime = (double)((NOW - LAST) * 1000 /
									(double)SDL_GetPerformanceFrequency());
		Uint32 frame_start = SDL_GetTicks();
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

		Uint32 frame_time = SDL_GetTicks() - frame_start;
		if (frame_time < FRAME_TARGET_TIME) {
			SDL_Delay(FRAME_TARGET_TIME - frame_time);
		}
	}

quit:
	card_images_vec_free(&cardImages);
	if (imageDb != NULL) {
		sqlite3_close(imageDb);
		imageDb = NULL;
	}
	if (cardDb != NULL) {
		sqlite3_close(cardDb);
		cardDb = NULL;
	}
	if (renderer != NULL) {
		SDL_DestroyRenderer(renderer);
		renderer = NULL;
	}
	if (window != NULL) {
		SDL_DestroyWindow(window);
		window = NULL;
	}
	if (robotoFont != NULL) {
		TTF_CloseFont(robotoFont);
		robotoFont = NULL;
	}
	free((void *)demoData.frameArena.memory);
	IMG_Quit();
	TTF_Quit();
	SDL_Quit();
	return returnVal;
}
