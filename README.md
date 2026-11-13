# Mtg Card search

Client to search magic the gathering cards using logic filters

## Resources used

- GUI library: [clay](https://github.com/nicbarker/clay)
- Window management using SDL2: [sdl](https://github.com/libsdl-org/SDL)
- Official sqlite library: [sqlite](https://www.sqlite.org/)
- Scryfall image [api bulk data](https://scryfall.com/docs/api/bulk-data) + [cdn](https://cards.scryfall.io/normal/front/0/0/0000419b-0bba-4488-8f7a-6194544ce91e.jpg?1783910776)
	- extracted, converted to avfi and packaged into an sqlite db


## Building

### Linux:

- Get the [dependencies](https://github.com/RodolphoVSantoro/mtg-card-search/releases/tag/dependencies) 
	- Put them under src/dependencies
- Download both the scryfall and mtgjson [databases](https://github.com/RodolphoVSantoro/mtg-card-search/releases/tag/latest)
	- Put both under resources
- run `make` or `make release` to build 
- run `make run` or `make run-release` to run the built application
