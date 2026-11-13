COMPILER=gcc

DEBUG_OUT=mtg-card-search-debug
RELEASE_OUT=mtg-card-search

SOURCE_FILES=src/main.c src/collections.c src/card_filter.c src/mtg_cards.c src/mtg_imgs.c
DEPENDENCY_SOURCE_FILES=src/dependencies/sqlite3.c src/dependencies/libavif.a
SYSTEM_LIBS=-lSDL2_ttf -lSDL2_image -lSDL2 -lm -lyuv -ldav1d
INCLUDES=-I/usr/include/SDL2

DEBUG_MEM_FLAGS=-fsanitize=address
DEBUG_FLAGS=-g
RELEASE_FLAGS=-Ofast
WARN_FLAGS=-Wall -Wextra -Werror -pedantic -fsyntax-only

STD_C=-std=c23

default:
	$(COMPILER) $(STD_C) $(DEBUG_FLAGS) -o $(DEBUG_OUT) $(SOURCE_FILES) $(DEPENDENCY_SOURCE_FILES) $(INCLUDES) $(SYSTEM_LIBS)

warn:
	$(COMPILER) $(STD_C) $(WARN_FLAGS) $(DEBUG_FLAGS) -o $(DEBUG_OUT) $(SOURCE_FILES) $(DEPENDENCY_SOURCE_FILES) $(INCLUDES) $(SYSTEM_LIBS)

release:
	$(COMPILER) $(STD_C) $(RELEASE_FLAGS) -o $(RELEASE_OUT) $(SOURCE_FILES) $(DEPENDENCY_SOURCE_FILES) $(INCLUDES) $(SYSTEM_LIBS)

run:
	./$(DEBUG_OUT)

run-release:
	./$(RELEASE_OUT)
