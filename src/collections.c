#include "collections.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char* card_face_as_str(CardFace cardFace) {
	if(cardFace == CARD_FACE_FRONT){
		return CARD_FACE_FRONT_STR;
	}
	if(cardFace == CARD_FACE_BACK) {
		return CARD_FACE_BACK_STR;
	}
	return CARD_FACE_UNKNOWN_STR;
}

CardFace card_face_from_cstring(const char *cardFaceStr) {
	if(strncmp("front", cardFaceStr, 5) == 0) {
		return CARD_FACE_FRONT;
	}
	if(strncmp("back", cardFaceStr, 4) == 0) {
		return CARD_FACE_BACK;
	}
	return CARD_FACE_UNKNOWN;
}

bool card_face_validate(CardFace face) {
	if (face != CARD_FACE_FRONT && face != CARD_FACE_BACK) {
		return false;
	}
	return true;
}

CardImages card_images_init(String cardId) {
	String ownedId = string_init();
	string_copy(&ownedId, cardId);
	return (CardImages){
		.cardId = ownedId,
		.faces = {NULL, NULL},
	};
}

CardImagesVec card_images_vec_init() { return (CardImagesVec){NULL, 0, 0}; }

int card_images_find_by_id(CardImagesVec vec, String id) {
	for (int i = 0; i < vec.size; i++) {
		if (string_equal(vec.cardsImages[i].cardId, id)) {
			return i;
		}
	}
	return CARD_NOT_FOUND;
}

int card_images_vec_push(CardImagesVec *vec, SDL_Texture *texture, String id,
						 CardFace face) {
	if (!card_face_validate(face)) {
		fprintf(stderr, "invalid card face in image vec push %d\n", face);
		return 1;
	}
	if (vec->size == vec->capacity) {
		vec->capacity = (vec->capacity + 1) * 2;
		vec->cardsImages = realloc_raw_array(vec->cardsImages, vec->capacity);
	}
	if (vec->cardsImages == NULL) {
		fprintf(stderr, "failed to realloc image vec to capacity %d\n", vec->capacity);
		return 1;
	}
	int position = card_images_find_by_id(*vec, id);
	if (position != CARD_NOT_FOUND) {
		if (vec->cardsImages[position].faces[face] != NULL) {
			fprintf(stderr, "trying to ovewrite sdl texture for id %s, face: %d\n", string_as_cstring(id), face);
			return 1;
		}
		vec->cardsImages[position].faces[face] = texture;
	} else {
		CardImages cardImages = card_images_init(id);
		cardImages.faces[face] = texture;
		vec->cardsImages[vec->size] = cardImages;
		vec->size++;
	}
	return 0;
}

void card_images_vec_free(CardImagesVec *vec) {
	for (int i = 0; i < vec->size; i++) {
		if (vec->cardsImages->faces[CARD_FACE_FRONT] != NULL) {
			SDL_DestroyTexture(vec->cardsImages->faces[CARD_FACE_FRONT]);
			vec->cardsImages->faces[CARD_FACE_FRONT] = NULL;
		}
		if (vec->cardsImages->faces[CARD_FACE_BACK] != NULL) {
			SDL_DestroyTexture(vec->cardsImages->faces[CARD_FACE_BACK]);
			vec->cardsImages->faces[CARD_FACE_BACK] = NULL;
		}
		string_free(&vec->cardsImages->cardId);
	}
	if(vec->cardsImages != NULL) {
		free(vec->cardsImages);
	}
	vec->cardsImages = NULL;
	vec->size = vec->capacity = 0;
}

CStringVec cstring_vec_init() { return (CStringVec){NULL, 0, 0}; }

char *_cstring_from_str(char *str) {
	int size = strlen(str);
	char* cstring = (char*)malloc(sizeof(char) * (size + 1));
	strcpy(cstring, str);
	return cstring;
}

int cstring_vec_push(CStringVec *vec, char *string) {
	if (vec->size == vec->capacity) {
		vec->capacity = (vec->capacity + 1) * 2;
		vec->strings = realloc_raw_array(vec->strings, vec->capacity);
	}
	if (vec->strings == NULL) {
		return 1;
	}
	vec->strings[vec->size] = string;
	vec->size++;
	return 0;
}

void cstring_vec_free(CStringVec *vec) {
	for (int i = 0; i < vec->size; i++) {
		free(vec->strings[i]);
		vec->strings[i] = NULL;
	}
	free(vec->strings);
	vec->strings = NULL;
	vec->size = vec->capacity = 0;
}

String string_init() {
	char *terminator = (char *)malloc(sizeof(char) * STRING_INITIAL_SIZE);
	memset(terminator, '\0', STRING_INITIAL_SIZE);
	return (String){terminator, 0, STRING_INITIAL_SIZE};
}

int string_push_cstring(String *string, char *cstring) {
	int cstringSize = strlen(cstring);
	if (cstringSize + string->size >= string->capacity) {
		string->capacity = (cstringSize + string->size) * 2;
		string->characters =
			realloc_raw_array(string->characters, string->capacity);
	}
	if (string->characters == NULL) {
		return 1;
	}
	strcat(string->characters, cstring);
	string->size = strlen(string->characters);
	return 0;
}

int string_copy(String *destination, String source) {
	if (source.size > destination->capacity) {
		destination->capacity = (source.size) * 2;
		destination->characters =
			realloc_raw_array(destination->characters, destination->capacity);
	}
	if (destination->characters == NULL) {
		return 1;
	}
	strcpy(destination->characters, source.characters);
	destination->size = source.size;
	return 0;
}

bool string_equal(String s1, String s2) {
	if (s1.size != s2.size) {
		return false;
	}
	for (int i = 0; i < s1.size; i++) {
		if (s1.characters[i] != s2.characters[i]) {
			return false;
		}
	}
	return true;
}

void string_reset(String *str) {
	str->size = 0;
	str->characters[0] = '\0';
}

char *string_as_cstring(String str) { return str.characters; }

void string_free(String *vec) {
	free(vec->characters);
	vec->characters = NULL;
	vec->size = vec->capacity = 0;
}
