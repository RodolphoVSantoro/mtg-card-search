#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "clay_renderer_SDL2.h"
#include "sdl/SDL.h"
#include "sdl/SDL_image.h"
#include "sdl/SDL_render.h"
#include "sdl/SDL_ttf.h"

#include "sdl/SDL_video.h"
#include "sqlite3.h"

#include "collections.h"
#include "mtg_card_search_layout.h"
#include "mtg_cards.h"
#include "mtg_imgs.h"

#define CLAY_IMPLEMENTATION
#include "clay.h"

constexpr int TARGET_FPS = 240;
constexpr int FRAME_TARGET_TIME = 1000 / TARGET_FPS;

constexpr int MAIN_ERROR = -1;
constexpr int MAIN_SUCCESS = 0;

constexpr char CARD_DB_FILE_NAME[] = "./resources/all_cards.db";
constexpr char IMAGE_DB_FILE_NAME[] = "./resources/scryfall_images.db";

int
sqlite_init(sqlite3 **imageDb, sqlite3 **cardDb);
int
sdl_init(SDL_Renderer **renderer, SDL_Window **window,
		 SDL2Font fonts[CLAY_FONT_AMOUNT]);
ClayData
clay_init(SDL_Renderer *renderer, SDL_Window *window,
		  SDL2Font fonts[CLAY_FONT_AMOUNT], CardImagesVec *cardImages);
int
event_loop(SDL_Renderer *renderer, SDL_Window *window,
		   SDL2Font fonts[CLAY_FONT_AMOUNT], ClayData *clayData);

int
main(int argc, char *argv[]) {
	int mainResult = MAIN_SUCCESS;

	CardImagesVec cardImages = card_images_vec_init();

	sqlite3 *imageDb = nullptr;
	sqlite3 *cardDb = nullptr;

	SDL_Renderer *renderer = nullptr;
	SDL_Window *window = nullptr;
	SDL2Font fonts[CLAY_FONT_AMOUNT] = {};
	fonts[FONT_ID_BODY_16].fontId = FONT_ID_BODY_16;
	fonts[FONT_ID_BODY_16].font = nullptr;

	CardFilter cardFilter = card_filter_create_group(CARD_FILTER_AND);

	ClayData clayData = {.queryBuffer = nullptr};

	mainResult = sqlite_init(&imageDb, &cardDb);
	if (mainResult != 0) {
		goto quit;
	}

	mainResult = sdl_init(&renderer, &window, fonts);
	if (mainResult != 0) {
		goto quit;
	}

	// create mock filter for now
	// TODO: get from UI
	// CardFilterEqual equal = {.key = "manaValue", .value = "3"};
	// CardFilterLike like = {.key = "name", .value = "phoenix"};
	// CardFilter filters[2] = {
	// 	{.type = CARD_FILTER_EQUAL, .filter = {.equalFilter = &equal}},
	// 	{.type = CARD_FILTER_LIKE, .filter = {.likeFilter = &like}}};
	// CardFilterAnd filterAnd =
	// 	(CardFilterAnd){.filtersSize = 2, .filters = filters};
	// CardFilter filter =
	// 	(CardFilter){.type = CARD_FILTER_AND, .filter = &filterAnd};

	// card_search(cardDb, imageDb, renderer, &filter, &cardImages);

	clayData = clay_init(renderer, window, fonts, &cardImages);

	clayData.cardFilter = &cardFilter;
	FilterUIState filterState = {
		.treeDirty = false,
		.activeEditNode = &cardFilter,
		.rootFilter = cardFilter,
		.editingValue = false,
		.pendingAction = FILTER_ACTION_NONE,
	};
	clayData.filterUIState = &filterState;
	clayData.imageDb = imageDb;
	clayData.cardDb = cardDb;

	mainResult = event_loop(renderer, window, fonts, &clayData);

quit:
	card_images_vec_free(&cardImages);
	card_filter_free_contents(&cardFilter);
	if (clayData.queryBuffer != nullptr) {
		free(clayData.queryBuffer);
	}
	if (imageDb != nullptr) {
		sqlite3_close(imageDb);
		imageDb = nullptr;
	}
	if (cardDb != nullptr) {
		sqlite3_close(cardDb);
		cardDb = nullptr;
	}
	if (renderer != nullptr) {
		SDL_DestroyRenderer(renderer);
		renderer = nullptr;
	}
	if (window != nullptr) {
		SDL_DestroyWindow(window);
		window = nullptr;
	}
	TTF_Font *robotoFont = fonts[FONT_ID_BODY_16].font;
	if (robotoFont != nullptr) {
		TTF_CloseFont(robotoFont);
		robotoFont = nullptr;
	}
	IMG_Quit();
	TTF_Quit();
	SDL_Quit();

	return mainResult;
}

int
sqlite_init(sqlite3 **imageDb, sqlite3 **cardDb) {
	int error = sqlite3_open(CARD_DB_FILE_NAME, cardDb);
	if (error != SQLITE_OK) {
		fprintf(stderr, "Cannot open card database: %s(%d)\n",
				sqlite3_errmsg(*cardDb), error);
		return MAIN_ERROR;
	}

	error = sqlite3_open(IMAGE_DB_FILE_NAME, imageDb);
	if (error != SQLITE_OK) {
		fprintf(stderr, "Cannot open image database: %s(%d)\n",
				sqlite3_errmsg(*imageDb), error);
		return MAIN_ERROR;
	}
	return MAIN_SUCCESS;
}

int
sdl_init(SDL_Renderer **renderer, SDL_Window **window,
		 SDL2Font fonts[CLAY_FONT_AMOUNT]) {
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "Error: could not initialize SDL: %s\n",
				SDL_GetError());
		return MAIN_ERROR;
	}
	if (TTF_Init() < 0) {
		fprintf(stderr, "Error: could not initialize TTF: %s\n",
				TTF_GetError());
		return MAIN_ERROR;
	}
	if (IMG_Init(IMG_INIT_PNG) < 0) {
		fprintf(stderr, "Error: could not initialize IMG: %s\n",
				IMG_GetError());
		return MAIN_ERROR;
	}

	fonts[FONT_ID_BODY_16].font =
		TTF_OpenFont("resources/Roboto-Regular.ttf", CLAY_FONT_SIZE);
	if (fonts[FONT_ID_BODY_16].font == nullptr) {
		fprintf(stderr, "Error: could not load font: %s\n", TTF_GetError());
		return MAIN_ERROR;
	}

	SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl"); // for antialiasing
	*window = SDL_CreateWindow("SDL", SDL_WINDOWPOS_UNDEFINED,
							   SDL_WINDOWPOS_UNDEFINED, 900, 600,
							   SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

	if (window == NULL) {
		fprintf(stderr, "Failed to init sdl window\n");
		return MAIN_ERROR;
	}
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4); // for antialiasing

	*renderer = SDL_CreateRenderer(
		*window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	if (renderer == NULL) {
		fprintf(stderr, "Failed to init sdl renderer\n");
		return MAIN_ERROR;
	}
	SDL_SetRenderDrawBlendMode(*renderer,
							   SDL_BLENDMODE_BLEND); // for alpha blending
	return MAIN_SUCCESS;
}

ClayData
clay_init(SDL_Renderer *renderer, SDL_Window *window,
		  SDL2Font fonts[CLAY_FONT_AMOUNT], CardImagesVec *cardImages) {
	uint64_t totalMemorySize = Clay_MinMemorySize();
	Clay_Arena clayMemory = Clay_CreateArenaWithCapacityAndMemory(
		totalMemorySize, malloc(totalMemorySize));

	int windowWidth = 0;
	int windowHeight = 0;
	SDL_GetWindowSize(window, &windowWidth, &windowHeight);
	Clay_Initialize(clayMemory,
					(Clay_Dimensions){(float)windowWidth, (float)windowHeight},
					(Clay_ErrorHandler){handle_clay_errors, NULL});
	Clay_SetMeasureTextFunction(measure_text, fonts);

	char *queryBuffer = (char *)malloc(sizeof(char) * MAX_QUERY_LEN);
	memset(queryBuffer, '\0', MAX_QUERY_LEN - 1);

	ClayData userData = {.window = window,
						 .windowWidth = windowWidth,
						 .windowHeight = windowHeight,
						 .cardImages = cardImages,
						 .renderer = renderer,
						 .fonts = fonts,
						 .queryBuffer = queryBuffer,
						 .queryLen = 0,
						 .queryFocused = false,
						 .sliderValue = 0.5,
						 .draggingSlider = false};

	return userData;
}

int
event_loop(SDL_Renderer *renderer, SDL_Window *window, SDL2Font *fonts,
		   ClayData *clayData) {
	SDL_AddEventWatch(resize_rendering, clayData);

	Uint64 NOW = SDL_GetPerformanceCounter();
	bool running = true;
	int result = MAIN_SUCCESS;

	while (running == true) {
		Uint64 LAST = NOW;
		Uint64 NOW = SDL_GetPerformanceCounter();
		double deltaTime = (double)((NOW - LAST) * 1000 /
									(double)SDL_GetPerformanceFrequency());
		Uint32 frame_start = SDL_GetTicks();
		Clay_Vector2 scrollDelta = {};
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (clayData->filterUIState->activeEditNode != NULL) {
				char **targetStr = NULL;
				if (clayData->filterUIState->activeEditNode->type ==
					CARD_FILTER_EQUAL) {
					targetStr = &clayData->filterUIState->activeEditNode->filter
									 .equalFilter->value;
				} else if (clayData->filterUIState->activeEditNode->type ==
						   CARD_FILTER_LIKE) {
					targetStr = &clayData->filterUIState->activeEditNode->filter
									 .likeFilter->value;
				}

				if (targetStr != NULL) {
					if (event.type == SDL_TEXTINPUT) {
						size_t oldLen = strlen(*targetStr);
						size_t addLen = strlen(event.text.text);
						*targetStr = realloc(*targetStr, oldLen + addLen + 1);
						strcat(*targetStr, event.text.text);
						clayData->filterUIState->treeDirty = true;
					} else if (event.type == SDL_KEYDOWN &&
							   event.key.keysym.sym == SDLK_BACKSPACE) {
						size_t len = strlen(*targetStr);
						if (len > 0) {
							(*targetStr)[len - 1] = '\0';
							clayData->filterUIState->treeDirty = true;
						}
					} else if (event.type == SDL_KEYDOWN &&
							   event.key.keysym.sym == SDLK_RETURN) {
						clayData->filterUIState->activeEditNode =
							NULL; // Commit/unfocus on Enter
					}
				}
			}
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
			case SDL_MOUSEMOTION:
				clayData->mouseX = (float)event.motion.x;
				clayData->mouseY = (float)event.motion.y - clayData->yOffset;
				break;
			case SDL_MOUSEBUTTONDOWN:
				if (event.button.button == SDL_BUTTON_LEFT) {
					clayData->mouseDown = true;
					clayData->mouseClicked = true;
				}
				break;
			case SDL_MOUSEBUTTONUP:
				if (event.button.button == SDL_BUTTON_LEFT) {
					clayData->mouseDown = false;
				}
				break;
			case SDL_TEXTINPUT:
				if (clayData->queryFocused) {
					size_t addLen = strlen(event.text.text);
					if (clayData->queryLen + addLen < MAX_QUERY_LEN - 1) {
						memcpy(clayData->queryBuffer + clayData->queryLen,
							   event.text.text, addLen);
						clayData->queryLen += (int)addLen;
						clayData->queryBuffer[clayData->queryLen] = '\0';
					}
				}
				break;
			case SDL_KEYDOWN:
				if (clayData->queryFocused &&
					event.key.keysym.sym == SDLK_BACKSPACE &&
					clayData->queryLen > 0) {
					clayData->queryLen--;
					clayData->queryBuffer[clayData->queryLen] = '\0';
				}
				break;
			default:
				break;
			}
		}

		int mouseX = 0;
		int mouseY = 0;
		Uint32 mouseState = SDL_GetMouseState(&mouseX, &mouseY);
		Clay_Vector2 mousePosition =
			(Clay_Vector2){(float)mouseX, (float)mouseY};
		Clay_SetPointerState(mousePosition, mouseState & SDL_BUTTON(1));

		Clay_SetPointerState((Clay_Vector2){clayData->mouseX, clayData->mouseY},
							 clayData->mouseDown);

		Clay_UpdateScrollContainers(
			false, (Clay_Vector2){scrollDelta.x, scrollDelta.y}, deltaTime);

		SDL_GetWindowSize(window, &clayData->windowWidth,
						  &clayData->windowHeight);
		Clay_SetLayoutDimensions((Clay_Dimensions){
			(float)clayData->windowWidth, (float)clayData->windowHeight});

		Clay_RenderCommandArray renderCommands =
			mtg_card_search_create_layout(clayData);
		result = SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		if (result != 0) {
			fprintf(stderr, "Failed to call sdl draw: %s\n", SDL_GetError());
			return result;
		}
		result = SDL_RenderClear(renderer);
		if (result != 0) {
			fprintf(stderr, "Failed to call sdl render clear: %s\n",
					SDL_GetError());
			return result;
		}

		clay_sdl2_render(renderer, renderCommands, fonts);
		SDL_RenderPresent(renderer);

		Uint32 frame_time = SDL_GetTicks() - frame_start;
		if (frame_time < FRAME_TARGET_TIME) {
			SDL_Delay(FRAME_TARGET_TIME - frame_time);
		}

		apply_pending_filter_action(clayData->filterUIState);
		// Reset mouse clicked at the end of the frame
		clayData->mouseClicked = false;
	}
	return result;
}
