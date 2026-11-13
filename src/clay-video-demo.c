#include "collections.h"
#include "dependencies/clay.h"
#include <SDL2/SDL.h>
#include <stdlib.h>

const int FONT_ID_BODY_16 = 0;
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

typedef struct {
	Clay_String title;
	Clay_String contents;
} Document;

typedef struct {
	Document *documents;
	uint32_t length;
} DocumentArray;

Document documentsRaw[5];

DocumentArray documents = {.length = 5, .documents = documentsRaw};

typedef struct {
	intptr_t offset;
	intptr_t memory;
} ClayVideoDemo_Arena;

typedef struct {
	float yOffset;
	ClayVideoDemo_Arena frameArena;
	CardImagesVec *cardImages;
} ClayVideoDemo_Data;

typedef struct {
	int32_t requestedDocumentIndex;
	int32_t *selectedDocumentIndex;
} SidebarClickData;

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

ClayVideoDemo_Data ClayVideoDemo_Initialize(CardImagesVec *cardImages) {
	ClayVideoDemo_Data data = {
		.cardImages = cardImages,
		.frameArena = {.memory = (intptr_t)malloc(1024)}};

	return data;
}

Clay_RenderCommandArray ClayVideoDemo_CreateLayout(ClayVideoDemo_Data *data) {
	data->frameArena.offset = 0;

	Clay_BeginLayout();

	Clay_Sizing layoutExpand = {.width = CLAY_SIZING_GROW(0),
								.height = CLAY_SIZING_GROW(0)};

	Clay_Color contentBackgroundColor = {15, 15, 40, 255};
	Clay_Color outerContentColor = {43, 41, 51, 128};
	Clay_Color borderColor = { 80, 40, 40, 255 };

	CLAY(CLAY_ID("OuterContainer"),
		 {.backgroundColor = outerContentColor,
		  .layout = {.layoutDirection = CLAY_TOP_TO_BOTTOM,
					 .sizing = layoutExpand,
					 .padding = CLAY_PADDING_ALL(16),
					 .childGap = 16}}) {
		CLAY(CLAY_ID("LowerContent"),
			 {.layout = {.sizing = layoutExpand, .childGap = 16}}) {
			CLAY(CLAY_ID("Sidebar"),
				 {.backgroundColor = contentBackgroundColor,
				  .layout = {.layoutDirection = CLAY_TOP_TO_BOTTOM,
							 .padding = CLAY_PADDING_ALL(16),
							 .childGap = 8,
							 .sizing = {.width = CLAY_SIZING_FIXED(250),
										.height = CLAY_SIZING_GROW(0)}}}) {
				CLAY_TEXT(
					CLAY_STRING("Write query here"),
					CLAY_TEXT_CONFIG({.fontId = FONT_ID_BODY_16,
									  .fontSize = 20,
									  .textColor = {255, 255, 255, 255}}));
			}

			int totalItems = data->cardImages->size;
			float itemWidth = 300.0f;
			float itemHeight = 400.0f;
			float gap = 10.0f;
			float availableWidth = 1700.0f - 32.0f;
			int itemsPerRow = (int)((availableWidth + gap) / (itemWidth + gap));
			if (itemsPerRow < 1) {
				itemsPerRow = 1;
			}

			int totalRows = (totalItems + itemsPerRow - 1) / itemsPerRow;
			CLAY(CLAY_ID("MainContent"),
				 {
				  .backgroundColor = borderColor,
				  .layout =
					  {.sizing = layoutExpand,
					   .layoutDirection = CLAY_TOP_TO_BOTTOM,
					   .padding = CLAY_PADDING_ALL(16),
					   .childGap = gap},
				  .clip = {
					  .vertical = true,
					  .childOffset = Clay_GetScrollOffset()
				  }}) {

				for (int row = 0; row < totalRows; row++) {
					CLAY(CLAY_IDI("Row", row),
						 {
						 	.backgroundColor = borderColor,
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
								 {.layout = {.sizing = {.width =
															CLAY_SIZING_FIXED(
																itemWidth),
														.height =
															CLAY_SIZING_FIXED(
																itemHeight)}},
								  .backgroundColor = {0, 0, 0, 0},
								  .cornerRadius = CLAY_CORNER_RADIUS(8)}) {
								CLAY_AUTO_ID({
									.image = data->cardImages->cardsImages[i]
												 .faces[CARD_FACE_FRONT],
									.layout = {.sizing =
												   {
													   .width = itemWidth,
													   .height = itemHeight,
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
			data->yOffset;
	}
	return renderCommands;
}