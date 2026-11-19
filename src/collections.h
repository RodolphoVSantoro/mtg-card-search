#pragma once
#include "sdl/SDL_render.h"
#include <stdbool.h>

static constexpr int STRING_INITIAL_SIZE = 256;
static constexpr int CARD_NOT_FOUND = -1;

#define realloc_raw_array(ptr, capacity)                                       \
	(typeof(ptr))realloc((ptr), sizeof(*(ptr)) * (capacity))

#define return_if_string_error(result, string_ref)                             \
	do                                                                         \
		if (error != 0) {                                                      \
			string_free(string_ref);                                           \
			return error;                                                      \
		}                                                                      \
	while (0)

#define str_to_cstring(str) strdup("" str "")

typedef struct {
	char **strings;
	int size;
	int capacity;
} CStringVec;

typedef struct {
	char *characters;
	int size;
	int capacity;
} String;

static constexpr char CARD_FACES_STR[3][8] = {
	"front",
	"back",
	"unknown",
};

typedef enum {
	CARD_FACE_FRONT = 0,
	CARD_FACE_BACK = 1,
	CARD_FACE_UNKNOWN = 2,
} CardFace;

typedef struct {
	// one for each card face, defined on the above enum
	SDL_Texture *faces[2];
	String cardId;
} CardImages;

typedef struct {
	CardImages *cardsImages;
	int size;
	int capacity;
} CardImagesVec;

const char *
card_face_as_str(CardFace cardFace);
CardFace
str_as_card_face(const char *cardFaceStr);
CardImages
card_images_init(String cardId);
bool
card_face_validate(CardFace face);
CardImagesVec
card_images_vec_init();
void
card_images_vec_push(CardImagesVec *vec, SDL_Texture *surface, String id,
					 CardFace face);
int
card_images_find_by_id(CardImagesVec vec, String id);
void
card_images_vec_free(CardImagesVec *vec);
CStringVec
cstring_vec_init();
char *
str_to_lower(const char *str);
int
cstring_vec_push(CStringVec *vec, char *string);
void
cstring_vec_free(CStringVec *vec);
String
string_init();
int
string_push_str(String *string, const char *cstring);
int
string_push_char(String *string, char cChar);
char *
string_as_cstring(String str);
void
string_free(String *vec);
int
string_copy(String *destination, String source);
bool
string_equal(String s1, String s2);
void
string_reset(String *str);
