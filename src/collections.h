#pragma once
#include <SDL2/SDL_render.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

#define STRING_INITIAL_SIZE 128

#define realloc_raw_array(ptr, capacity)                                         \
	(typeof(ptr))realloc((ptr), sizeof(*(ptr)) * (capacity))

#define CARD_NOT_FOUND -1

#define return_if_string_error(result, string_ref) \
	do if(error != 0) { \
		string_free(string_ref); \
		return error; \
	} while(0)

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


static const char *CARD_FACE_FRONT_STR = "front";
static const char *CARD_FACE_BACK_STR = "back";
static const char *CARD_FACE_UNKNOWN_STR = "unknown";

typedef enum {
	CARD_FACE_UNKNOWN=-1,
	CARD_FACE_FRONT=0,
	CARD_FACE_BACK=1,
} CardFace;

typedef struct {
	// one for each card face, defined on the above enum
	SDL_Texture *faces[2];
	String cardId;
}CardImages;

typedef struct {
	CardImages *cardsImages;
	int size;
	int capacity;
} CardImagesVec;

#define cstring_from_str(str) _cstring_from_str("" str "")

const char* card_face_as_str(CardFace cardFace);
CardFace card_face_from_cstring(const char *cardFaceStr);
CardImages card_images_init(String cardId);
bool card_face_validate(CardFace face);
CardImagesVec card_images_vec_init();
int card_images_vec_push(CardImagesVec *vec, SDL_Texture *surface, String id, CardFace face);
int card_images_find_by_id(CardImagesVec vec, String id);
void card_images_vec_free(CardImagesVec *vec);
CStringVec cstring_vec_init();
char *_cstring_from_str(char *str);
int cstring_vec_push(CStringVec *vec, char *string);
void cstring_vec_free(CStringVec *vec);
String string_init();
int string_push_cstring(String *string, char *cstring);
char *string_as_cstring(String str);
void string_free(String *vec);
int string_copy(String *destination, String source);
bool string_equal(String s1, String s2);
void string_reset(String *str);