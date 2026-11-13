#include "collections.h"
#include "dependencies/avif.h"
#include "sqlite3.h"

#include "mtg_imgs.h"

#include <SDL2/SDL.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static int build_query(String *sql, CStringVec strs) {
	int error = string_push_cstring(sql, SQL_IMAGE_SELECT);
	return_if_string_error(result, sql);

	error = string_push_cstring(sql, SQL_IMAGE_START_WHERE);
	return_if_string_error(result, sql);

	for (int i = 0; i < strs.size; i++) {
		error = string_push_cstring(sql, "'");
		return_if_string_error(result, sql);

		error = string_push_cstring(sql, strs.strings[i]);
		return_if_string_error(result, sql);

		error = string_push_cstring(sql, "'");
		return_if_string_error(result, sql);
		if (i < strs.size - 1) {
			error = string_push_cstring(sql, ", ");
			return_if_string_error(result, sql);
		}
	}
	error = string_push_cstring(sql, SQL_IMAGE_CLOSE_WHERE);
	return_if_string_error(result, sql);
	return 0;
}

static SDL_Surface *create_surface_from_avif(const uint8_t *avifData,
									  size_t dataSize) {
	// 1. Initialize AVIF decoder
	avifDecoder *decoder = avifDecoderCreate();
	if (!decoder) {
		return NULL;
	}

	// 2. Read the memory buffer
	avifResult result = avifDecoderSetIOMemory(decoder, avifData, dataSize);
	if (result != AVIF_RESULT_OK) {
		avifDecoderDestroy(decoder);
		return NULL;
	}

	// 3. Decode the image
	result = avifDecoderParse(decoder);
	if (result != AVIF_RESULT_OK) {
		avifDecoderDestroy(decoder);
		return NULL;
	}

	result = avifDecoderNextImage(decoder);
	if (result != AVIF_RESULT_OK) {
		avifDecoderDestroy(decoder);
		return NULL;
	}

	// 4. Convert YUV/compressed data to raw RGB/RGBA pixels
	avifRGBImage rgbImage;
	avifRGBImageSetDefaults(&rgbImage, decoder->image);

	// SDL2 standard expects RGBA or ABGR depending on endianness;
	// AVIF_RGB_FORMAT_RGBA is widely compatible.
	rgbImage.format = AVIF_RGB_FORMAT_RGBA;
	rgbImage.depth = 8; // 8 bits per channel

	result = avifRGBImageAllocatePixels(&rgbImage);
	if (result != AVIF_RESULT_OK) {
		avifDecoderDestroy(decoder);
		return NULL;
	}

	result = avifImageYUVToRGB(decoder->image, &rgbImage);
	if (result != AVIF_RESULT_OK) {
		avifRGBImageFreePixels(&rgbImage);
		avifDecoderDestroy(decoder);
		return NULL;
	}

	// 5. Create an SDL_Surface from the raw RGB pixels
	// SDL_CreateRGBSurfaceFrom takes: pixels, width, height, depth, pitch,
	// RMask, GMask, BMask, AMask
	int width = rgbImage.width;
	int height = rgbImage.height;
	int pitch = rgbImage.rowBytes;

	// Handle endianness for masks
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	Uint32 rmask = 0xff000000;
	Uint32 gmask = 0x00ff0000;
	Uint32 bmask = 0x0000ff00;
	Uint32 amask = 0x000000ff;
#else
	Uint32 rmask = 0x000000ff;
	Uint32 gmask = 0x0000ff00;
	Uint32 bmask = 0x00ff0000;
	Uint32 amask = 0xff000000;
#endif
	SDL_Surface *surface =
		SDL_CreateRGBSurfaceFrom(rgbImage.pixels, width, height,
								 32,	// bits per pixel
								 pitch, // pitch (bytes per row)
								 rmask, gmask, bmask, amask);

	if (!surface) {
		avifRGBImageFreePixels(&rgbImage);
		avifDecoderDestroy(decoder);
		return NULL;
	}

	// Note: SDL_CreateRGBSurfaceFrom does NOT copy the pixel data; it creates a
	// view. If you want to free the libavif buffer immediately, you must
	// duplicate the surface: SDL_Surface* optimizedSurface =
	// SDL_ConvertSurface(surface, surface->format, 0);
	// SDL_FreeSurface(surface);
	// ... then free avif pixels.

	// Cleanup decoder structures (keep rgbImage.pixels allocated if using
	// SDL_CreateRGBSurfaceFrom directly)
	avifDecoderDestroy(decoder);

	return surface;
}

int get_card_blobs_by_id(sqlite3 *imageDb, CardImagesVec *cardImages,
						 CStringVec strs) {
	String sql = string_init();
	int error = build_query(&sql, strs);
	if (error != 0) {
		string_free(&sql);
		return error;
	}

	sqlite3_stmt *stmt;
	char *err_msg = NULL;
	error =
		sqlite3_prepare_v2(imageDb, string_as_cstring(sql), -1, &stmt, NULL);
	if (error != SQLITE_OK) {
		fprintf(stderr, "Failed to prepare statement: %s\n",
				sqlite3_errmsg(imageDb));
		sqlite3_finalize(stmt);
		return error;
	}

	int blob_size_sdl = 0;
	uint8_t *blob_data_sdl = NULL;
	String cardScryfallId = string_init();
	*cardImages = card_images_vec_init();
	while ((error = sqlite3_step(stmt)) == SQLITE_ROW) {
		char *cardFaceStr = (char *)sqlite3_column_text(stmt, 0);
		CardFace cardFace = card_face_from_cstring(cardFaceStr);
		char *cardScryfallIdStr = (char *)sqlite3_column_text(stmt, 1);
		error = string_push_cstring(&cardScryfallId, cardScryfallIdStr);
		if (error != 0) {
			break;
		}
		const void *blob_data = sqlite3_column_blob(stmt, 2);
		int blob_size = sqlite3_column_bytes(stmt, 2);
		blob_data_sdl = (uint8_t *)malloc(blob_size * sizeof(uint8_t));
		blob_size_sdl = blob_size;
		memcpy(blob_data_sdl, blob_data, blob_size);
		SDL_Surface *cardImage =
			create_surface_from_avif(blob_data_sdl, blob_size_sdl);
		// printf("Card face %s(%d), Card scryfallId: %s | img Size: %d bytes\n", cardFaceStr, cardFace, cardScryfallIdStr, blob_size);
		card_images_vec_push(cardImages, cardImage, cardScryfallId, cardFace);
		string_reset(&cardScryfallId);
	}
	string_free(&cardScryfallId);
	if (error != 0 && error != SQLITE_DONE) {
		fprintf(stderr, "Execution error: %s\n", sqlite3_errmsg(imageDb));
		card_images_vec_free(cardImages);
		sqlite3_finalize(stmt);
		return error;
	}

	sqlite3_finalize(stmt);
	return 0;
}
