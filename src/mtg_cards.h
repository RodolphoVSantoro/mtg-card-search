#pragma once

#include "collections.h"

#include <sqlite3.h>

static constexpr char SQL_CARD_SELECT_WHERE[] =
	"select ci.scryfallId from cards ca "
	"join cardIdentifiers ci on ci.uuid = ca.uuid "
	"where ";

static constexpr char SQL_CARD_GROUP_BY[] = " group by ca.name limit 100";

typedef struct {
	const char *uiKey;
	const char *sqlColumn;
} FilterKeyMapping;

static const FilterKeyMapping VALID_FILTER_KEYS[] = {
	{"name", "ca.name"},
	{"text", "ca.text"},
	{"flavorText", "ca.flavorText"},
	{"keywords", "ca.keywords"},
	{"manaCost", "ca.manaCost"},
	{"manaValue", "ca.manaValue"},
	{"life", "ca.life"},
	{"power", "ca.power"},
	{"toughness", "ca.toughness"},
	{"loyalty", "ca.loyalty"},
	{"colors", "ca.colors"},
	{"colorIdentity", "ca.colorIdentity"},
	{"type", "ca.type"},
	{"types", "ca.types"},
	{"subtypes", "ca.subtypes"},
	{"supertypes", "ca.supertypes"},
	{"borderColor", "ca.borderColor"},
	{"artist", "ca.artist"},
	{"leadershipSkills", "ca.leadershipSkills"},
	{"printings", "ca.printings"},
	{"scryfallId", "ci.scryfallId"},
};

#define VALID_FILTER_KEYS_COUNT                                                \
	(sizeof(VALID_FILTER_KEYS) / sizeof(VALID_FILTER_KEYS[0]))

const char *
get_sql_column_for_key(const char *uiKey);

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
	int filterCapacity;
	int filtersSize;
};

struct CardFilterOr {
	CardFilter *filters;
	int filterCapacity;
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

int
get_scryfall_ids(sqlite3 *cardDb, CardFilter filter, CStringVec *ids);
CardFilter
card_filter_create_leaf(CardFilterType type, const char *key, const char *val);
CardFilter
card_filter_create_group(CardFilterType type);
void
card_filter_free_contents(CardFilter *filter);
void
card_filter_group_add(CardFilter *group, CardFilter newChild);
void
card_filter_group_remove_at(CardFilter *group, int removeIdx);
