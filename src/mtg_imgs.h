#pragma once
#include "collections.h"
#include "sqlite3.h"

static constexpr char SQL_IMAGE_SELECT[] =
	"select card_face, card_id, image_data "
	"from mtg_images ";
static constexpr char SQL_IMAGE_START_WHERE[] = " where card_id in ( ";
static constexpr char SQL_IMAGE_CLOSE_WHERE[] = " )";

int
get_card_blobs_by_id(SDL_Renderer *renderer, sqlite3 *imageDb,
					 CardImagesVec *cardImages, CStringVec strs);
