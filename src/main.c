#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "sdl/SDL.h"
#include "sdl/SDL_image.h"
#include "sdl/SDL_ttf.h"

#include "sqlite3.h"

#include "collections.h"
#include "mtg_card_search_layout.h"
#include "mtg_cards.h"
#include "mtg_imgs.h"

#define CLAY_IMPLEMENTATION
#include "clay.h"

#define TARGET_FPS 240
#define FRAME_TARGET_TIME (1000 / TARGET_FPS)
#define MAIN_ERROR -1

int main(int argc, char *argv[]) {
	CardImagesVec cardImages = card_images_vec_init();
	CStringVec scryfallIds = cstring_vec_init();
	int returnVal = 0;
	sqlite3 *imageDb = NULL, *cardDb = NULL;

	// Sqlite init
	int error = sqlite3_open("./resources/all_cards.db", &cardDb);
	if (error != SQLITE_OK) {
		fprintf(stderr, "Cannot open card database: %s(%d)\n",
				sqlite3_errmsg(cardDb), error);
		returnVal = MAIN_ERROR;
		goto quit;
	}

	error = sqlite3_open("./resources/scryfall_images.db", &imageDb);
	if (error != SQLITE_OK) {
		fprintf(stderr, "Cannot open image database: %s(%d)\n",
				sqlite3_errmsg(imageDb), error);
		returnVal = MAIN_ERROR;
		goto quit;
	}
	// Sqlite initiated

	// SDL components init
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "Error: could not initialize SDL: %s\n",
				SDL_GetError());
		returnVal = MAIN_ERROR;
		goto quit;
	}
	if (TTF_Init() < 0) {
		fprintf(stderr, "Error: could not initialize TTF: %s\n",
				TTF_GetError());
		returnVal = MAIN_ERROR;
		goto quit;
	}
	if (IMG_Init(IMG_INIT_PNG) < 0) {
		fprintf(stderr, "Error: could not initialize IMG: %s\n",
				IMG_GetError());
		returnVal = MAIN_ERROR;
		goto quit;
	}

	TTF_Font *robotoFont = TTF_OpenFont("resources/Roboto-Regular.ttf", 16);
	if (!robotoFont) {
		fprintf(stderr, "Error: could not load font: %s\n", TTF_GetError());
		returnVal = MAIN_ERROR;
		goto quit;
	}

	SDL2_Font fonts[1] = {};

	fonts[FONT_ID_BODY_16] = (SDL2_Font){
		.fontId = FONT_ID_BODY_16,
		.font = robotoFont,
	};

	SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl"); // for antialiasing
	SDL_Window *window = SDL_CreateWindow(
		"SDL", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 900, 600,
		SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

	if (window == NULL) {
		fprintf(stderr, "Failed to init sdl window\n");
		returnVal = MAIN_ERROR;
		goto quit;
	}
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4); // for antialiasing

	SDL_Renderer *renderer = SDL_CreateRenderer(
		window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	if (renderer == NULL) {
		fprintf(stderr, "Failed to init sdl renderer\n");
		returnVal = MAIN_ERROR;
		goto quit;
	}
	SDL_SetRenderDrawBlendMode(renderer,
							   SDL_BLENDMODE_BLEND); // for alpha blending
	// SDL components initiated

	// Clay init
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

	// init card search
	CardFilterEqual equal = {.key = "manaValue", .value = "3"};
	CardFilterLike like = {.key = "name", .value = "phoenix"};
	CardFilter filters[2] = {
		{.type = CARD_FILTER_EQUAL, .filter = {.equalFilter = &equal}},
		{.type = CARD_FILTER_LIKE, .filter = {.likeFilter = &like}}};
	CardFilterAnd filterAnd =
		(CardFilterAnd){.filtersSize = 2, .filters = filters};

	CardFilter filter =
		(CardFilter){.type = CARD_FILTER_AND, .filter = &filterAnd};
	error = get_scryfall_ids(cardDb, filter, &scryfallIds);
	if (error != 0) {
		fprintf(stderr, "Failed to get scryfall ids: %d\n", error);
		goto quit;
	}

	get_card_blobs_by_id(renderer, imageDb, &cardImages, scryfallIds);
	ClayData demoData =
		CardSearchInitialize(&cardImages, &windowHeight, &windowWidth);

	cstring_vec_free(&scryfallIds);
	// end card search

	ResizeRenderData userData = {
		.window = window,			   // SDL_Window*
		.windowWidth = &windowWidth,   // int*
		.windowHeight = &windowHeight, // int*
		.demoData = demoData,		   // CustomShit
		.renderer = renderer,		   // SDL_Renderer*
		.fonts = fonts,				   // SDL2_Font[1]
	};
	// add an event watcher that will render the screen while youre dragging the
	// window to different sizes

	// Clay initiated

	// Event loop start
	SDL_AddEventWatch(resizeRendering, &userData);
	Uint64 NOW = SDL_GetPerformanceCounter();
	bool running = true;

	while (running == true) {
		Uint64 LAST = NOW;
		Uint64 NOW = SDL_GetPerformanceCounter();
		double deltaTime = (double)((NOW - LAST) * 1000 /
									(double)SDL_GetPerformanceFrequency());
		Uint32 frame_start = SDL_GetTicks();
		Clay_Vector2 scrollDelta = {};
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_WINDOWEVENT:
				if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
					running = false;
				}
				break;
			case SDL_QUIT:
				running = false;
				break;

			case SDL_MOUSEWHEEL:
				scrollDelta.x = event.wheel.x * 10;
				scrollDelta.y = event.wheel.y * 10;
				break;
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

	// Event loop end

	// cleanup
quit:
	cstring_vec_free(&scryfallIds);
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
	IMG_Quit();
	TTF_Quit();
	SDL_Quit();

	return returnVal;
}
