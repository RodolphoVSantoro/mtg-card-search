#pragma once

#include "clay.h"
#include "clay_renderer_SDL2.h"
#include "collections.h"
#include "mtg_cards.h"

static constexpr int CLAY_FONT_AMOUNT = 1;
static constexpr int CLAY_FONT_SIZE = 16;
static constexpr int FONT_ID_BODY_16 = 0;

static constexpr int MAX_QUERY_LEN = 4096;

typedef enum {
	FILTER_ACTION_NONE,
	FILTER_ACTION_ADD_LEAF,
	FILTER_ACTION_ADD_GROUP,
	FILTER_ACTION_REMOVE_CHILD,
	FILTER_ACTION_TOGGLE_GROUP_TYPE,
	FILTER_ACTION_WRAP_IN_NOT,
	FILTER_ACTION_SET_LEAF_KEY,
} FilterActionType;

typedef struct {
	FilterActionType type;
	CardFilter *targetGroup;
	CardFilter *targetNode;
	int childIndex;
	CardFilterType newType;
	const char *newKey;
} PendingFilterAction;

typedef struct {
	CardFilter rootFilter;
	CardFilter *activeEditNode;
	CardFilter *openKeyDropdownNode;
	bool editingValue;
	PendingFilterAction pendingAction;
	bool treeDirty;
} FilterUIState;

typedef struct {
	// *** Sqlite *** //
	sqlite3 *imageDb;
	sqlite3 *cardDb;

	// *** Window management *** //
	SDL_Renderer *renderer;
	SDL2Font *fonts;
	SDL_Window *window;
	int windowWidth;
	int windowHeight;
	float yOffset;

	// *** Mouse state *** //
	float mouseX;
	float mouseY;
	bool mouseDown;	   // Currently held down
	bool mouseClicked; // True only on the frame the button was pressed

	// *** Filter *** //
	CardFilter *cardFilter;
	bool cardFilterUpdated;

	// *** Card Images *** //
	CardImagesVec *cardImages;

	// *** Query management *** //
	char *queryBuffer;
	int queryLen;
	bool queryFocused;

	// *** filter tree management *** //
	FilterUIState *filterUIState;

	float sliderValue; // card scale: 0.1f to 10.0f
	bool draggingSlider;
} ClayData;

int
resize_rendering(void *userData, SDL_Event *event);
Clay_RenderCommandArray
mtg_card_search_create_layout(ClayData *userData);
void
handle_clay_errors(Clay_ErrorData errorData);
void
apply_pending_filter_action(FilterUIState *state);
int
card_search(sqlite3 *cardDb, sqlite3 *imageDb, SDL_Renderer *renderer,
			CardFilter *filter, CardImagesVec *cardImages);