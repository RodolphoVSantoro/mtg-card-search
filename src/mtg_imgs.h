#pragma once
#include "collections.h"
#include "sqlite3.h"

#define SQL_IMAGE_SELECT "select card_face, card_id, image_data from mtg_images "
#define SQL_IMAGE_START_WHERE " where card_id in ( "
#define SQL_IMAGE_CLOSE_WHERE " )"

int get_card_blobs_by_id(sqlite3 *imageDb, CardImagesVec *cardImages,
						 CStringVec strs);
