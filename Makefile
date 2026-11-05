default:
	gcc -o mtg-card-search-debug src/main.c -fsanitize=address -g -I/usr/include/SDL2 -lSDL2_ttf -lSDL2_image -lSDL2 -lm

release:
	gcc -Ofast -o mtg-card-search src/main.c -I/usr/include/SDL2 -lSDL2_ttf -lSDL2_image -lSDL2 -lm

run:
	./mtg-card-search-debug

run-release:
	./mtg-card-search
