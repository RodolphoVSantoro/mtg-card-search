#pragma once

#include "collections.h"

#include <sqlite3.h>

#define SQL_CARD_SELECT_WHERE                                                  \
	"select ci.scryfallId from cards ca "                                      \
	"join cardIdentifiers ci on ci.uuid = ca.uuid "                            \
	"where "

#define SQL_CARD_GROUP_BY " group by ca.name"

typedef enum {
	CARD_FILTER_AND,
	CARD_FILTER_OR,
	CARD_FILTER_NOT,
	CARD_FILTER_LIKE,
	CARD_FILTER_EQUAL,
} CardFilterType;

typedef struct CardFilterAnd CardFilterAnd;
typedef struct CardFilterOr CardFilterOr;
typedef struct CardFilterNot CardFilterNot;
typedef struct CardFilterNot CardFilterNot;
typedef struct CardFilterEqual CardFilterEqual;
typedef struct CardFilterLike CardFilterLike;

typedef union {
	CardFilterAnd *andFilter;
	CardFilterOr *orFilter;
	CardFilterNot *notFilter;
	CardFilterEqual *equalFilter;
	CardFilterLike *likeFilter;
} CardFilterUnion;

typedef struct {
	CardFilterType type;
	CardFilterUnion filter;
} CardFilter;

struct CardFilterNot {
	CardFilter filter;
};

struct CardFilterAnd {
	CardFilter *filters;
	int filtersSize;
};

struct CardFilterOr {
	CardFilter *filters;
	int filtersSize;
};

typedef union {
	char *stringValue;
	int intValue;
	double doubleValue;
} FilterValueUnion;

struct CardFilterEqual {
	char *key;
	char *value;
};

struct CardFilterLike {
	char *key;
	char *value;
};

int get_scryfall_ids(sqlite3 *cardDb, CardFilter filter, CStringVec *ids);
