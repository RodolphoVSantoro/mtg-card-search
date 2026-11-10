#pragma once
#include "collections.h"
#include "sqlite3.h"

#define SQL_IMAGE_SELECT "select card_face, card_id, image_data from images "
#define SQL_IMAGE_START_WHERE " where filename in ( "
#define SQL_IMAGE_CLOSE_WHERE " )"

int get_card_blobs_by_id(sqlite3 *imageDb, CardImagesVec *surfaces, CStringVec strs);
SDL_Surface *create_surface_from_avif(const uint8_t *avifData, size_t dataSize);