#include "mtg_card_search_layout.h"

#include <math.h>
#include <stdlib.h>

Document documentsRaw[5];
DocumentArray documents = {.length = 5, .documents = documentsRaw};

Clay_Color COLOR_WHITE = {255, 255, 255, 255};

void RenderHeaderButton(Clay_String text) {
	CLAY_AUTO_ID({.layout = {.padding = {16, 16, 8, 8}},
				  .backgroundColor = {140, 140, 140, 255},
				  .cornerRadius = CLAY_CORNER_RADIUS(5)}) {
		CLAY_TEXT(text, CLAY_TEXT_CONFIG({.fontId = FONT_ID_BODY_16,
										  .fontSize = 16,
										  .textColor = {255, 255, 255, 255}}));
	}
}

void RenderDropdownMenuItem(Clay_String text) {
	CLAY_AUTO_ID({.layout = {.padding = CLAY_PADDING_ALL(16)}}) {
		CLAY_TEXT(text, CLAY_TEXT_CONFIG({.fontId = FONT_ID_BODY_16,
										  .fontSize = 16,
										  .textColor = {255, 255, 255, 255}}));
	}
}

void HandleSidebarInteraction(Clay_ElementId elementId,
							  Clay_PointerData pointerData, void *userData) {
	SidebarClickData *clickData = (SidebarClickData *)userData;
	// If this button was clicked
	if (pointerData.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
		if (clickData->requestedDocumentIndex >= 0 &&
			clickData->requestedDocumentIndex < documents.length) {
			// Select the corresponding document
			*clickData->selectedDocumentIndex =
				clickData->requestedDocumentIndex;
		}
	}
}

ClayVideoDemo_Data ClayVideoDemo_Initialize(CardImagesVec *cardImages,
											int *windowHeight,
											int *windowWidth) {
	ClayVideoDemo_Data userData = {
		.windowHeight = windowHeight,
		.windowWidth = windowWidth,
		.cardImages = cardImages,
		.frameArena = {.memory = (intptr_t)malloc(1024)}};

	return userData;
}

int apply_proportion(float size, float proportion) {
	return (int)ceil(size * proportion);
}

Clay_RenderCommandArray
ClayVideoDemo_CreateLayout(ClayVideoDemo_Data *userData) {
	userData->frameArena.offset = 0;

	// default size w: 1920, h: 1006
	float widthProportion = ((float)(*(userData->windowWidth))) / 1920.0f;
	float heightProportion = ((float)(*(userData->windowHeight))) / 1006.0f;

	Clay_BeginLayout();

	Clay_Sizing layoutExpand = {.width = CLAY_SIZING_GROW(0),
								.height = CLAY_SIZING_GROW(0)};

	Clay_Color contentBackgroundColor = {15, 15, 40, 255};
	Clay_Color outerContentColor = {43, 41, 51, 128};
	Clay_Color borderColor = {80, 40, 40, 255};

	CLAY(CLAY_ID("OuterContainer"),
		 {.backgroundColor = outerContentColor,
		  .layout = {.layoutDirection = CLAY_TOP_TO_BOTTOM,
					 .sizing = layoutExpand,
					 .padding = CLAY_PADDING_ALL(
						 apply_proportion(16.0f, heightProportion)),
					 .childGap = apply_proportion(16.0f, heightProportion)}}) {
		CLAY(CLAY_ID("LowerContent"),
			 {.layout = {.sizing = layoutExpand,
						 .childGap =
							 apply_proportion(16.0f, heightProportion)}}) {
			CLAY(CLAY_ID("Sidebar"),
				 {.backgroundColor = contentBackgroundColor,
				  .layout = {
					  .layoutDirection = CLAY_TOP_TO_BOTTOM,
					  .padding = CLAY_PADDING_ALL(
						  apply_proportion(16.0f, heightProportion)),
					  .childGap = apply_proportion(8.0f, heightProportion),
					  .sizing = {.width = CLAY_SIZING_FIXED(
									 apply_proportion(250.0f, widthProportion)),
								 .height = CLAY_SIZING_GROW(0)}}}) {
				CLAY_TEXT(
					CLAY_STRING("Write query here"),
					CLAY_TEXT_CONFIG(
						{.fontId = FONT_ID_BODY_16,
						 .fontSize = apply_proportion(20.0f, widthProportion),
						 .textColor = {255, 255, 255, 255}}));
			}

			int totalItems = userData->cardImages->size;

			// 4:3 ratio for card size
			float baseCardWidth = 375.f;
			float baseCardHeight = 500.0f;
			// yes, they are both supposed to increase by height proportion
			// increasing the window width should only increase the amount of
			// cards per row
			float mtgCardWidth =
				apply_proportion(baseCardWidth, heightProportion);
			float mtgCardHeight =
				apply_proportion(baseCardHeight, heightProportion);

			float gap = apply_proportion(10.0f, heightProportion);
			float availableWidth = apply_proportion(1600.0f, widthProportion);
			int itemsPerRow =
				(int)((availableWidth + gap) / (mtgCardWidth + gap));
			if (itemsPerRow < 1) {
				itemsPerRow = 1;
			}

			int totalRows = (totalItems + itemsPerRow - 1) / itemsPerRow;
			CLAY(CLAY_ID("MainContent"),
				 {.backgroundColor = borderColor,
				  .layout = {.sizing = layoutExpand,
							 .layoutDirection = CLAY_TOP_TO_BOTTOM,
							 .padding = CLAY_PADDING_ALL(
								 apply_proportion(16.0f, widthProportion)),
							 .childGap = gap},
				  .clip = {.vertical = true,
						   .childOffset = Clay_GetScrollOffset()}}) {

				for (int row = 0; row < totalRows; row++) {
					CLAY(CLAY_IDI("Row", row),
						 {.backgroundColor = borderColor,
						  .layout = {
							  .layoutDirection = CLAY_LEFT_TO_RIGHT,
							  .childGap = gap,
							  .sizing = {.width = CLAY_SIZING_GROW(0),
										 .height = CLAY_SIZING_FIT(0)}}}) {
						int startIndex = row * itemsPerRow;
						int endIndex = startIndex + itemsPerRow;
						if (endIndex > totalItems)
							endIndex = totalItems;

						for (int i = startIndex; i < endIndex; i++) {
							CLAY(CLAY_IDI("GridItem", i),
								 {.layout =
									  {.sizing = {.width = CLAY_SIZING_FIXED(
													  mtgCardWidth),
												  .height = CLAY_SIZING_FIXED(
													  mtgCardHeight)}},
								  .backgroundColor = {0, 0, 0, 0},
								  .cornerRadius =
									  CLAY_CORNER_RADIUS(apply_proportion(
										  8.0f, widthProportion))}) {
								CLAY_AUTO_ID({
									.image =
										userData->cardImages->cardsImages[i]
											.faces[CARD_FACE_FRONT],
									.layout = {.sizing =
												   {
													   .width = mtgCardWidth,
													   .height = mtgCardHeight,
												   }},
								});
							}
						}
					}
				}
			}
		}
	}

	Clay_RenderCommandArray renderCommands = Clay_EndLayout(0);
	for (int32_t i = 0; i < renderCommands.length; i++) {
		Clay_RenderCommandArray_Get(&renderCommands, i)->boundingBox.y +=
			userData->yOffset;
	}
	return renderCommands;
}

int resizeRendering(void *userData, SDL_Event *event) {
	ResizeRenderData *actualData = userData;
	if (event->type == SDL_WINDOWEVENT &&
		event->window.event == SDL_WINDOWEVENT_EXPOSED) {
		SDL_Window *window = actualData->window;
		int windowWidth = *(actualData->windowWidth);
		int windowHeight = *(actualData->windowHeight);
		ClayVideoDemo_Data demoData = actualData->demoData;
		SDL_Renderer *renderer = actualData->renderer;
		SDL2_Font *fonts = actualData->fonts;

		SDL_GetWindowSize(window, &windowWidth, &windowHeight);
		Clay_SetLayoutDimensions(
			(Clay_Dimensions){(float)windowWidth, (float)windowHeight});

		*(actualData->windowWidth) = windowWidth;
		*(actualData->windowHeight) = windowHeight;

		Clay_RenderCommandArray renderCommands =
			ClayVideoDemo_CreateLayout(&demoData);
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);

		Clay_SDL2_Render(renderer, renderCommands, fonts);

		SDL_RenderPresent(renderer);
	}
	return 0;
}
