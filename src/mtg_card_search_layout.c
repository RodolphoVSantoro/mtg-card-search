#include "mtg_card_search_layout.h"
#include "mtg_cards.h"
#include "mtg_imgs.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

// ******* Layout colors ******** //
static constexpr Clay_Color contentBackgroundColor = {15, 15, 40, 255};
static constexpr Clay_Color outerContentColor = {43, 41, 51, 128};
static constexpr Clay_Color borderColor = {80, 40, 40, 255};
static constexpr Clay_Color COLOR_WHITE = {255, 255, 255, 255};
static constexpr Clay_Color SELECTED_COLOR = {90, 90, 160, 255};
static constexpr Clay_Color HOVERED_COLOR = {65, 65, 110, 255};
static constexpr Clay_Color UNSELECTED_COLOR = {35, 35, 65, 255};
static constexpr Clay_Color QUERY_SELECTED_COLOR = {35, 35, 75, 255};
static constexpr Clay_Color QUERY_UNSELECTED_COLOR = {25, 25, 55, 255};

// ******* Cards base size ******** //
static constexpr float CARD_BASE_SIZE = 125.f;
// 4:3 ratio for card size
static constexpr float BASE_CARD_WIDTH = 3.f * CARD_BASE_SIZE;	// 3 * 125 = 375
static constexpr float BASE_CARD_HEIGHT = 4.f * CARD_BASE_SIZE; // 4 * 125 = 500

int
card_search(sqlite3 *cardDb, sqlite3 *imageDb, SDL_Renderer *renderer,
			CardFilter *filter, CardImagesVec *cardImages) {
	CStringVec scryfallIds = cstring_vec_init();

	int mainResult = get_scryfall_ids(cardDb, *filter, &scryfallIds);
	if (mainResult != 0) {
		fprintf(stderr, "Failed to get scryfall ids: %d\n", mainResult);
		cstring_vec_free(&scryfallIds);
		return mainResult;
	}

	mainResult =
		get_card_blobs_by_id(renderer, imageDb, cardImages, scryfallIds);
	cstring_vec_free(&scryfallIds);
	return mainResult;
}

void
handle_clay_errors(Clay_ErrorData errorData) {
	printf("%s", errorData.errorText.chars);
}

static int
apply_proportion(float size, float proportion) {
	return (int)ceil(size * proportion);
}

// Helper for interactive buttons with hover feedback
static bool
clay_button(Clay_ElementId id, Clay_String label, float widthProportion,
			float heightProportion, bool isActive, bool mouseClicked) {
	bool hovered = Clay_PointerOver(id);
	Clay_Color bgColor = isActive  ? SELECTED_COLOR
						 : hovered ? HOVERED_COLOR
								   : UNSELECTED_COLOR;

	CLAY(id,
		 {.backgroundColor = bgColor,
		  .cornerRadius =
			  CLAY_CORNER_RADIUS(apply_proportion(6.0f, widthProportion)),
		  .layout = {.padding =
						 {
							 .left = apply_proportion(12.0f, widthProportion),
							 .right = apply_proportion(12.0f, widthProportion),
							 .top = apply_proportion(8.0f, heightProportion),
							 .bottom = apply_proportion(8.0f, heightProportion),
						 },
					 .childAlignment = {.x = CLAY_ALIGN_X_CENTER,
										.y = CLAY_ALIGN_Y_CENTER}}}) {
		CLAY_TEXT(label,
				  CLAY_TEXT_CONFIG({
					  .fontId = FONT_ID_BODY_16,
					  .fontSize = apply_proportion(16.0f, widthProportion),
					  .textColor = {255, 255, 255, 255},
				  }));
	}

	return hovered && mouseClicked;
}

static void
mtg_card_search_create_filter_layout(
	CardFilter *node,
	CardFilter *parentGroup, // nullptr for root
	int indexInParent, FilterUIState *state, float widthProportion,
	float heightProportion, int *uiIndex, bool mouseClicked) {
	bool isLeaf =
		(node->type == CARD_FILTER_LIKE || node->type == CARD_FILTER_EQUAL);
	Clay_LayoutDirection dir = isLeaf ? CLAY_LEFT_TO_RIGHT : CLAY_TOP_TO_BOTTOM;

	// Subtle visual nesting: darken or tint background based on type
	Clay_Color containerBg =
		isLeaf ? (Clay_Color){45, 48, 56, 255} : (Clay_Color){30, 32, 38, 255};

	CLAY(CLAY_IDI("FilterNode", (*uiIndex)++),
		 {.backgroundColor = containerBg,
		  .cornerRadius =
			  CLAY_CORNER_RADIUS(apply_proportion(8.0f, widthProportion)),
		  .border = {.width = CLAY_BORDER_ALL(1), .color = {80, 85, 95, 255}},
		  .layout = {.layoutDirection = dir,
					 .padding = CLAY_PADDING_ALL(
						 apply_proportion(8.0f, widthProportion)),
					 .childGap = apply_proportion(6.0f, widthProportion),
					 .childAlignment = {.y = CLAY_ALIGN_Y_CENTER},
					 .sizing = {.width = CLAY_SIZING_GROW(0),
								.height = CLAY_SIZING_FIT(0)}}}) {
		if (isLeaf) {
			char *key = (node->type == CARD_FILTER_EQUAL)
							? node->filter.equalFilter->key
							: node->filter.likeFilter->key;
			char *val = (node->type == CARD_FILTER_EQUAL)
							? node->filter.equalFilter->value
							: node->filter.likeFilter->value;
			bool isDropdownOpen = (state->openKeyDropdownNode == node);
			bool isEditingValue = (state->activeEditNode == node);

			// 1. Key Selector Button + Floating Dropdown
			Clay_ElementId keyBtnId = CLAY_IDI("LeafKey", (*uiIndex)++);
			Clay_String keyStr = {.length = (int)strlen(key), .chars = key};

			if (clay_button(keyBtnId, keyStr, widthProportion, heightProportion,
							isDropdownOpen, mouseClicked)) {
				state->openKeyDropdownNode = isDropdownOpen ? NULL : node;
				state->activeEditNode =
					NULL; // Stop text input while picking a key
			}

			if (isDropdownOpen) {
				CLAY(
					CLAY_IDI("KeyDropdown", (*uiIndex)++),
					{.backgroundColor = {25, 27, 33, 255},
					 .cornerRadius = CLAY_CORNER_RADIUS(
						 apply_proportion(6.0f, widthProportion)),
					 .border = {.width = CLAY_BORDER_ALL(1),
								.color = {110, 115, 130, 255}},
					 .floating = {.attachTo = CLAY_ATTACH_TO_ELEMENT_WITH_ID,
								  .parentId = keyBtnId.id,
								  .attachPoints =
									  {.element = CLAY_ATTACH_POINT_LEFT_TOP,
									   .parent = CLAY_ATTACH_POINT_LEFT_BOTTOM},
								  .zIndex = 100},
					 .clip = {.vertical = true,
							  .childOffset = Clay_GetScrollOffset()},
					 .layout = {
						 .layoutDirection = CLAY_TOP_TO_BOTTOM,
						 .padding = CLAY_PADDING_ALL(
							 apply_proportion(4.0f, widthProportion)),
						 .childGap = apply_proportion(2.0f, widthProportion),
						 .sizing = {
							 .width = CLAY_SIZING_FIXED(
								 apply_proportion(160.0f, widthProportion)),
							 .height = CLAY_SIZING_FIXED(apply_proportion(
								 220.0f, heightProportion)) // Scrollable list
						 }}}) {
					for (size_t k = 0; k < VALID_FILTER_KEYS_COUNT; k++) {
						Clay_String optLabel = {
							.length = (int)strlen(VALID_FILTER_KEYS[k].uiKey),
							.chars = VALID_FILTER_KEYS[k].uiKey};
						bool isCurrentKey =
							(strcmp(key, VALID_FILTER_KEYS[k].uiKey) == 0);

						if (clay_button(CLAY_IDI("KeyOpt", (*uiIndex)++),
										optLabel, widthProportion,
										heightProportion, isCurrentKey,
										mouseClicked)) {
							state->pendingAction = (PendingFilterAction){
								.type = FILTER_ACTION_SET_LEAF_KEY,
								.targetNode = node,
								.newKey = VALID_FILTER_KEYS[k].uiKey};
						}
					}
				}
			}

			// 2. Operator Toggle Button (= vs LIKE)
			Clay_String opStr = (node->type == CARD_FILTER_EQUAL)
									? CLAY_STRING("EQUAL")
									: CLAY_STRING("LIKE");
			if (clay_button(CLAY_IDI("LeafOp", (*uiIndex)++), opStr,
							widthProportion, heightProportion, false,
							mouseClicked)) {
				if (node->type == CARD_FILTER_EQUAL) {
					CardFilterLike *like = malloc(sizeof(CardFilterLike));
					like->key = key;
					like->value = val;
					free(node->filter.equalFilter);
					node->type = CARD_FILTER_LIKE;
					node->filter.likeFilter = like;
				} else {
					CardFilterEqual *eq = malloc(sizeof(CardFilterEqual));
					eq->key = key;
					eq->value = val;
					free(node->filter.likeFilter);
					node->type = CARD_FILTER_EQUAL;
					node->filter.equalFilter = eq;
				}
				state->treeDirty = true;
			}

			// 3. Value Input Button (click to type value)
			Clay_String valStr =
				strlen(val) > 0
					? (Clay_String){.length = (int)strlen(val), .chars = val}
					: CLAY_STRING("<type value>");
			if (clay_button(CLAY_IDI("LeafVal", (*uiIndex)++), valStr,
							widthProportion, heightProportion, isEditingValue,
							mouseClicked)) {
				state->activeEditNode = node;
				state->openKeyDropdownNode = NULL;
			}

			// 4. Wrap in NOT ("!") & Remove ("X") buttons ...
			if (clay_button(CLAY_IDI("WrapNot", (*uiIndex)++), CLAY_STRING("!"),
							widthProportion, heightProportion, false,
							mouseClicked)) {
				state->pendingAction = (PendingFilterAction){
					.type = FILTER_ACTION_WRAP_IN_NOT, .targetNode = node};
			}

			// 5. Remove button (only if inside an AND/OR group)
			if (parentGroup != NULL) {
				if (clay_button(CLAY_IDI("DelNode", (*uiIndex)++),
								CLAY_STRING("X"), widthProportion,
								heightProportion, false, mouseClicked)) {
					state->pendingAction = (PendingFilterAction){
						.type = FILTER_ACTION_REMOVE_CHILD,
						.targetGroup = parentGroup,
						.childIndex = indexInParent};
				}
			}
		} else if (node->type == CARD_FILTER_AND ||
				   node->type == CARD_FILTER_OR) {
			// --- Group Header Row ---
			CLAY(
				CLAY_IDI("GroupHeader", (*uiIndex)++),
				{.layout = {.layoutDirection = CLAY_LEFT_TO_RIGHT,
							.childGap = apply_proportion(6.0f, widthProportion),
							.childAlignment = {.y = CLAY_ALIGN_Y_CENTER}}}) {
				// Toggle AND / OR
				Clay_String groupLabel = (node->type == CARD_FILTER_AND)
											 ? CLAY_STRING("AND (All)")
											 : CLAY_STRING("OR (Any)");
				if (clay_button(CLAY_IDI("ToggleGroup", (*uiIndex)++),
								groupLabel, widthProportion, heightProportion,
								true, mouseClicked)) {
					state->pendingAction = (PendingFilterAction){
						.type = FILTER_ACTION_TOGGLE_GROUP_TYPE,
						.targetNode = node};
				}

				// "+ Rule" button (adds a leaf inside this layer)
				if (clay_button(CLAY_IDI("AddRule", (*uiIndex)++),
								CLAY_STRING("+ Rule"), widthProportion,
								heightProportion, false, mouseClicked)) {
					state->pendingAction =
						(PendingFilterAction){.type = FILTER_ACTION_ADD_LEAF,
											  .targetGroup = node,
											  .newType = CARD_FILTER_LIKE};
				}

				// "+ Group" button (adds a nested sub-layer)
				if (clay_button(CLAY_IDI("AddGroup", (*uiIndex)++),
								CLAY_STRING("+ Group"), widthProportion,
								heightProportion, false, mouseClicked)) {
					state->pendingAction = (PendingFilterAction){
						.type = FILTER_ACTION_ADD_GROUP,
						.targetGroup = node,
						.newType = (node->type == CARD_FILTER_AND)
									   ? CARD_FILTER_OR
									   : CARD_FILTER_AND};
				}

				// Remove entire group button (if not root)
				if (parentGroup != NULL) {
					if (clay_button(CLAY_IDI("DelGroup", (*uiIndex)++),
									CLAY_STRING("X"), widthProportion,
									heightProportion, false, mouseClicked)) {
						state->pendingAction = (PendingFilterAction){
							.type = FILTER_ACTION_REMOVE_CHILD,
							.targetGroup = parentGroup,
							.childIndex = indexInParent};
					}
				}
			}

			// --- Render Children Recursively ---
			CardFilter *children = (node->type == CARD_FILTER_AND)
									   ? node->filter.andFilter->filters
									   : node->filter.orFilter->filters;
			int count = (node->type == CARD_FILTER_AND)
							? node->filter.andFilter->filtersSize
							: node->filter.orFilter->filtersSize;
			if (count == 0) {
				CLAY_TEXT(
					CLAY_STRING("(Empty group - click + Rule or + Group)"),
					CLAY_TEXT_CONFIG({
						.fontId = FONT_ID_BODY_16,
						.fontSize = apply_proportion(14.0f, widthProportion),
						.textColor = {150, 155, 165, 255},
					}));
			}
			for (int i = 0; i < count; i++) {
				mtg_card_search_create_filter_layout(
					&children[i], node, i, state, widthProportion,
					heightProportion, uiIndex, mouseClicked);
			}
		} else if (node->type == CARD_FILTER_NOT) {
			CLAY(CLAY_IDI("NotHeader", (*uiIndex)++),
				 {.layout = {.layoutDirection = CLAY_LEFT_TO_RIGHT,
							 .childGap =
								 apply_proportion(6.0f, widthProportion)}}) {
				CLAY_TEXT(
					CLAY_STRING("NOT"),
					CLAY_TEXT_CONFIG({
						.fontId = FONT_ID_BODY_16,
						.fontSize = apply_proportion(16.0f, widthProportion),
						.textColor = {255, 120, 120, 255},
					}));

				// Unwrap NOT button (replaces NOT node with its own child)
				if (clay_button(CLAY_IDI("UnwrapNot", (*uiIndex)++),
								CLAY_STRING("Unwrap"), widthProportion,
								heightProportion, false, mouseClicked)) {
					CardFilter childCopy = node->filter.notFilter->filter;
					free(node->filter.notFilter);
					*node = childCopy;
					state->treeDirty = true;
					return; // Return immediately since node was replaced
				}
			}

			mtg_card_search_create_filter_layout(
				&node->filter.notFilter->filter, NULL, 0, state,
				widthProportion, heightProportion, uiIndex, mouseClicked);
		}
	}
}

void
apply_pending_filter_action(FilterUIState *state) {
	PendingFilterAction action = state->pendingAction;
	if (action.type == FILTER_ACTION_NONE)
		return;

	switch (action.type) {
	case FILTER_ACTION_ADD_LEAF: {
		CardFilter leaf = card_filter_create_leaf(action.newType, "name", "");
		card_filter_group_add(action.targetGroup, leaf);
		break;
	}
	case FILTER_ACTION_ADD_GROUP: {
		CardFilter grp = card_filter_create_group(action.newType);
		card_filter_group_add(action.targetGroup, grp);
		break;
	}
	case FILTER_ACTION_SET_LEAF_KEY: {
		CardFilter *n = action.targetNode;
		if (n->type == CARD_FILTER_EQUAL) {
			free(n->filter.equalFilter->key);
			n->filter.equalFilter->key = strdup(action.newKey);
		} else if (n->type == CARD_FILTER_LIKE) {
			free(n->filter.likeFilter->key);
			n->filter.likeFilter->key = strdup(action.newKey);
		}
		state->openKeyDropdownNode = NULL;
		break;
	}
	case FILTER_ACTION_REMOVE_CHILD: {
		if (state->activeEditNode == &action.targetGroup->filter.andFilter
										  ->filters[action.childIndex] ||
			state->openKeyDropdownNode == &action.targetGroup->filter.andFilter
											   ->filters[action.childIndex]) {
			state->activeEditNode = NULL;
			state->openKeyDropdownNode = NULL;
		}
		card_filter_group_remove_at(action.targetGroup, action.childIndex);
		break;
	}
	case FILTER_ACTION_TOGGLE_GROUP_TYPE: {
		CardFilter *node = action.targetNode;
		if (node->type == CARD_FILTER_AND) {
			CardFilterOr *orFilter = malloc(sizeof(CardFilterOr));
			orFilter->filters = node->filter.andFilter->filters;
			orFilter->filtersSize = node->filter.andFilter->filtersSize;
			orFilter->filterCapacity = node->filter.andFilter->filterCapacity;
			free(node->filter.andFilter);
			node->type = CARD_FILTER_OR;
			node->filter.orFilter = orFilter;
		} else if (node->type == CARD_FILTER_OR) {
			CardFilterAnd *andF = malloc(sizeof(CardFilterAnd));
			andF->filters = node->filter.orFilter->filters;
			andF->filtersSize = node->filter.orFilter->filtersSize;
			andF->filterCapacity = node->filter.orFilter->filterCapacity;
			free(node->filter.orFilter);
			node->type = CARD_FILTER_AND;
			node->filter.andFilter = andF;
		}
		break;
	}
	case FILTER_ACTION_WRAP_IN_NOT: {
		CardFilter original = *action.targetNode;
		CardFilterNot *notFilter = malloc(sizeof(CardFilterNot));
		notFilter->filter = original;
		action.targetNode->type = CARD_FILTER_NOT;
		action.targetNode->filter.notFilter = notFilter;
		break;
	}
	default:
		break;
	}

	state->pendingAction.type = FILTER_ACTION_NONE;
	state->treeDirty = true;
}

static void
mtg_card_search_create_sidebar_layout(ClayData *clayData, float widthProportion,
									  float heightProportion) {
	Clay_Color inputBgColor =
		clayData->queryFocused ? QUERY_SELECTED_COLOR : QUERY_UNSELECTED_COLOR;
	CLAY(CLAY_ID("SidebarMenu"),
		 {.layout = {.layoutDirection = CLAY_LEFT_TO_RIGHT,
					 .childGap = apply_proportion(6.0f, widthProportion),
					 .sizing = {.width = CLAY_SIZING_GROW(0),
								.height = CLAY_SIZING_FIT(0)}}}) {
		// clay_button(CLAY_ID("TabFilters"), CLAY_STRING("Filters"),
		// 			widthProportion, heightProportion, false,
		// 			clayData->mouseClicked);
		if (clay_button(CLAY_ID("TabSearch"), CLAY_STRING("Search"),
						widthProportion, heightProportion, false,
						clayData->mouseClicked)) {
			card_search(clayData->cardDb, clayData->imageDb, clayData->renderer,
						clayData->cardFilter, clayData->cardImages);
		}
	}
	int indexProportion = 0;
	int parentId = 0;
	CardFilter *parentNode = nullptr;
	mtg_card_search_create_filter_layout(
		clayData->cardFilter, parentNode, parentId, clayData->filterUIState,
		widthProportion, heightProportion, &indexProportion,
		clayData->mouseClicked);
}

static void
mtg_card_search_create_cards_layout(ClayData *clayData, float widthProportion,
									float heightProportion) {
	// ******* Cards Sizing ******* //
	// yes, they are both supposed to increase by height proportion
	// increasing the window width should only increase the amount of
	// cards per row
	float sliderProportion = 2.0f * (clayData->sliderValue + 0.1f);

	float mtgCardWidth =
		apply_proportion(BASE_CARD_WIDTH, heightProportion * sliderProportion);
	float mtgCardHeight =
		apply_proportion(BASE_CARD_HEIGHT, heightProportion * sliderProportion);

	// ******* Cards Flex Wrap ******* //
	int totalCards = clayData->cardImages->size;
	float gap = apply_proportion(10.0f, heightProportion);
	float cardRowWidth = apply_proportion(1600.0f, widthProportion);
	int cardsPerRow = (int)((cardRowWidth + gap) / (mtgCardWidth + gap));
	if (cardsPerRow < 1) {
		cardsPerRow = 1;
	}

	int totalRows = (totalCards + cardsPerRow - 1) / cardsPerRow;

	CLAY(CLAY_ID("CardsMenuBar"),
		 {.layout = {.layoutDirection = CLAY_LEFT_TO_RIGHT,
					 .sizing = {.width = CLAY_SIZING_GROW(0),
								.height = CLAY_SIZING_FIT(0)},
					 .childGap = apply_proportion(24.0f, widthProportion),
					 .childAlignment = {.y = CLAY_ALIGN_Y_CENTER}}}) {

		float sliderWidth = apply_proportion(200.0f, widthProportion);
		float sliderHeight = apply_proportion(12.0f, heightProportion);
		float knobSize = apply_proportion(18.0f, heightProportion);
		Clay_ElementId sliderId = CLAY_ID("CardScaleSlider");

		if (clayData->mouseClicked && Clay_PointerOver(sliderId)) {
			clayData->draggingSlider = true;
		}
		if (!clayData->mouseDown) {
			clayData->draggingSlider = false;
		}
		if (clayData->draggingSlider) {
			Clay_ElementData sliderData = Clay_GetElementData(sliderId);
			if (sliderData.found && sliderData.boundingBox.width > 0) {
				float t = (clayData->mouseX - sliderData.boundingBox.x) /
						  sliderData.boundingBox.width;
				if (t < 0.0f) {
					t = 0.0f;
				}
				if (t > 1.0f) {
					t = 1.0f;
				}
				clayData->sliderValue = t;
			}
		}

		CLAY(sliderId,
			 {.backgroundColor = {30, 30, 50, 255},
			  .cornerRadius = CLAY_CORNER_RADIUS(sliderHeight * 0.5f),
			  .layout = {.sizing = {.width = CLAY_SIZING_FIXED(sliderWidth),
									.height = CLAY_SIZING_FIXED(sliderHeight)},
						 .childAlignment = {.y = CLAY_ALIGN_Y_CENTER}}}) {
			CLAY(
				CLAY_ID("SliderFill"),
				{.backgroundColor = {110, 110, 210, 255},
				 .cornerRadius = CLAY_CORNER_RADIUS(sliderHeight * 0.5f),
				 .layout = {.sizing = {.width = CLAY_SIZING_FIXED(
										   sliderWidth * clayData->sliderValue),
									   .height = CLAY_SIZING_GROW(0)}}});

			CLAY(CLAY_ID("SliderKnob"),
				 {.backgroundColor = {230, 230, 255, 255},
				  .cornerRadius = CLAY_CORNER_RADIUS(knobSize * 0.5f),
				  .layout = {
					  .sizing = {.width = CLAY_SIZING_FIXED(knobSize),
								 .height = CLAY_SIZING_FIXED(knobSize)}}});
		}

		CLAY(CLAY_ID("CardActionButtons"),
			 {.layout = {.layoutDirection = CLAY_LEFT_TO_RIGHT,
						 .childGap = apply_proportion(8.0f, widthProportion),
						 .sizing = {.width = CLAY_SIZING_FIT(0),
									.height = CLAY_SIZING_FIT(0)}}}) {
			clay_button(CLAY_ID("BtnShowImages"),
						CLAY_STRING("Show card images"), widthProportion,
						heightProportion, false, clayData->mouseClicked);
			clay_button(CLAY_ID("BtnShowData"), CLAY_STRING("Show card data"),
						widthProportion, heightProportion, false,
						clayData->mouseClicked);
		}
	}
	for (int row = 0; row < totalRows; row++) {
		CLAY(CLAY_IDI("Row", row),
			 {.backgroundColor = borderColor,
			  .layout = {.layoutDirection = CLAY_LEFT_TO_RIGHT,
						 .childGap = gap,
						 .sizing = {.width = CLAY_SIZING_GROW(0),
									.height = CLAY_SIZING_FIT(0)}}}) {
			int startIndex = row * cardsPerRow;
			int endIndex = startIndex + cardsPerRow;
			if (endIndex > totalCards) {
				endIndex = totalCards;
			}

			for (int i = startIndex; i < endIndex; i++) {
				CLAY(CLAY_IDI("GridItem", i),
					 {.layout = {.sizing = {.width =
												CLAY_SIZING_FIXED(mtgCardWidth),
											.height = CLAY_SIZING_FIXED(
												mtgCardHeight)}},
					  .backgroundColor = {0, 0, 0, 0},
					  .cornerRadius = CLAY_CORNER_RADIUS(
						  apply_proportion(8.0f, widthProportion))}) {
					CLAY_AUTO_ID({
						.image = clayData->cardImages->cardsImages[i]
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

Clay_RenderCommandArray
mtg_card_search_create_layout(ClayData *clayData) {
	// ******* Proportion of layout relative to window ******* //
	// default size w: 1920, h: 1006
	float widthProportion = ((float)(clayData->windowWidth)) / 1920.0f;
	float heightProportion = ((float)(clayData->windowHeight)) / 1006.0f;

	// ******* Other layout settings ******* //
	Clay_Sizing layoutExpand = {.width = CLAY_SIZING_GROW(0),
								.height = CLAY_SIZING_GROW(0)};
	float gap = apply_proportion(10.0f, heightProportion);

	Clay_BeginLayout();

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
									 apply_proportion(450.0f, widthProportion)),
								 .height = CLAY_SIZING_GROW(0)}}}) {
				mtg_card_search_create_sidebar_layout(clayData, widthProportion,
													  heightProportion);
			}

			CLAY(CLAY_ID("MainContent"),
				 {.backgroundColor = borderColor,
				  .layout = {.sizing = layoutExpand,
							 .layoutDirection = CLAY_TOP_TO_BOTTOM,
							 .padding = CLAY_PADDING_ALL(
								 apply_proportion(16.0f, widthProportion)),
							 .childGap = gap},
				  .clip = {.vertical = true,
						   .childOffset = Clay_GetScrollOffset()}}) {
				mtg_card_search_create_cards_layout(clayData, widthProportion,
													heightProportion);
			}
		}
	}

	Clay_RenderCommandArray renderCommands = Clay_EndLayout(0);
	for (int32_t i = 0; i < renderCommands.length; i++) {
		Clay_RenderCommandArray_Get(&renderCommands, i)->boundingBox.y +=
			clayData->yOffset;
	}
	return renderCommands;
}

int
resize_rendering(void *userData, SDL_Event *event) {
	ClayData *actualData = userData;
	if (event->type == SDL_WINDOWEVENT &&
		event->window.event == SDL_WINDOWEVENT_EXPOSED) {
		SDL_Window *window = actualData->window;
		int windowWidth = actualData->windowWidth;
		int windowHeight = actualData->windowHeight;
		SDL_Renderer *renderer = actualData->renderer;
		SDL2Font *fonts = actualData->fonts;

		SDL_GetWindowSize(window, &windowWidth, &windowHeight);
		Clay_SetLayoutDimensions(
			(Clay_Dimensions){(float)windowWidth, (float)windowHeight});

		actualData->windowWidth = windowWidth;
		actualData->windowHeight = windowHeight;

		Clay_RenderCommandArray renderCommands =
			mtg_card_search_create_layout(actualData);
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);

		clay_sdl2_render(renderer, renderCommands, fonts);

		SDL_RenderPresent(renderer);
	}
	return 0;
}
