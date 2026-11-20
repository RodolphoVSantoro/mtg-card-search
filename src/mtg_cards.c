#include "mtg_cards.h"
#include "collections.h"

#include <stdio.h>

static String
apply_filter(CardFilter filter);

static String
generate_query(CardFilter filter) {
	String sql = string_init();
	int error = string_push_str(&sql, SQL_CARD_SELECT_WHERE);
	if (error != 0) {
		fprintf(stderr, "Failed to alloc sql card query\n");
		exit(1);
	}
	String queryFilter = apply_filter(filter);
	error = string_push_str(&sql, string_as_cstring(queryFilter));
	string_free(&queryFilter);
	if (error != 0) {
		fprintf(stderr, "Failed to alloc sql card query\n");
		exit(1);
	}
	error = string_push_str(&sql, SQL_CARD_GROUP_BY);
	if (error != 0) {
		fprintf(stderr, "Failed to alloc sql card query\n");
		exit(1);
	}
	return sql;
}

static String
apply_and_filter(CardFilterAnd filter) {
	String query = string_init();
	if (filter.filtersSize <= 0) {
		return query;
	}
	int error = string_push_str(&query, "( ");
	if (error != 0) {
		fprintf(stderr, "Failed to alloc sql and filter '('\n");
		exit(1);
	}
	for (int i = 0; i < filter.filtersSize; i++) {
		String subQuery = apply_filter(filter.filters[i]);
		error = string_push_str(&query, string_as_cstring(subQuery));
		if (error != 0) {
			fprintf(stderr, "Failed to alloc sql and filter subquery\n");
			exit(1);
		}
		string_free(&subQuery);
		if (i < filter.filtersSize - 1) {
			error = string_push_str(&query, " and ");
			if (error != 0) {
				fprintf(stderr, "Failed to alloc sql and filter 'and'\n");
				exit(1);
			}
		}
	}
	error = string_push_str(&query, ") ");
	if (error != 0) {
		fprintf(stderr, "Failed to alloc sql and filter ')'\n");
		exit(1);
	}
	return query;
}

static String
apply_or_filter(CardFilterOr filter) {
	String query = string_init();
	if (filter.filtersSize <= 0) {
		return query;
	}
	int error = string_push_str(&query, " ( ");
	if (error != 0) {
		fprintf(stderr, "Failed to alloc sql or filter '('\n");
		exit(1);
	}
	for (int i = 0; i < filter.filtersSize; i++) {
		String subQuery = apply_filter(filter.filters[i]);
		error = string_push_str(&query, string_as_cstring(subQuery));
		if (error != 0) {
			fprintf(stderr, "Failed to alloc sql or filter subquery\n");
			exit(1);
		}
		string_free(&subQuery);
		if (i < filter.filtersSize - 1) {
			error = string_push_str(&query, " or ");
			if (error != 0) {
				fprintf(stderr, "Failed to alloc sql or filter 'or'\n");
				exit(1);
			}
		}
	}
	error = string_push_str(&query, " ) ");
	if (error != 0) {
		fprintf(stderr, "Failed to alloc sql or filter ')'\n");
		exit(1);
	}
	return query;
}

static String
apply_not_filter(CardFilterNot filter) {
	String query = string_init();
	int error = string_push_str(&query, " not ( ");
	if (error != 0) {
		fprintf(stderr, "Failed to alloc sql and filter '('\n");
		exit(1);
	}
	String subQuery = apply_filter(filter.filter);
	error = string_push_str(&query, string_as_cstring(subQuery));
	if (error != 0) {
		fprintf(stderr, "Failed to alloc sql and filter subquery\n");
		exit(1);
	}
	string_free(&subQuery);
	error = string_push_str(&query, " ) ");
	if (error != 0) {
		fprintf(stderr, "Failed to alloc sql and filter ')'\n");
		exit(1);
	}
	return query;
}

static String
apply_equal_filter(CardFilterEqual filter) {
	String query = string_init();
	int error = string_push_str(&query, filter.key);
	if (error != 0) {
		fprintf(stderr, "Failed to alloc sql and filter subquery\n");
		exit(1);
	}
	error = string_push_str(&query, " = '");
	if (error != 0) {
		fprintf(stderr, "Failed to alloc sql and filter ')'\n");
		exit(1);
	}
	error = string_push_str(&query, filter.value);
	if (error != 0) {
		fprintf(stderr, "Failed to alloc sql and filter subquery\n");
		exit(1);
	}
	error = string_push_str(&query, "' ");
	if (error != 0) {
		fprintf(stderr, "Failed to alloc sql and filter ')'\n");
		exit(1);
	}
	return query;
}

static String
apply_like_filter(CardFilterLike filter) {
	String query = string_init();
	int error = string_push_str(&query, " lower(");
	if (error != 0) {
		fprintf(stderr, "Failed to alloc sql and filter subquery\n");
		exit(1);
	}
	error = string_push_str(&query, filter.key);
	if (error != 0) {
		fprintf(stderr, "Failed to alloc sql and filter subquery\n");
		exit(1);
	}
	error = string_push_str(&query, ") ");
	if (error != 0) {
		fprintf(stderr, "Failed to alloc sql and filter subquery\n");
		exit(1);
	}
	error = string_push_str(&query, " like '%");
	if (error != 0) {
		fprintf(stderr, "Failed to alloc sql and filter ')'\n");
		exit(1);
	}
	char *valueToLower = str_to_lower(filter.value);
	error = string_push_str(&query, valueToLower);
	free(valueToLower);
	if (error != 0) {
		fprintf(stderr, "Failed to alloc sql and filter subquery\n");
		exit(1);
	}
	error = string_push_str(&query, "%' ");
	if (error != 0) {
		fprintf(stderr, "Failed to alloc sql and filter ')'\n");
		exit(1);
	}
	return query;
}

static String
apply_filter(CardFilter filter) {
	switch (filter.type) {
	case CARD_FILTER_AND: {
		CardFilterAnd andFilter = *filter.filter.andFilter;
		return apply_and_filter(andFilter);
	}
	case CARD_FILTER_OR: {
		CardFilterOr orFilter = *filter.filter.orFilter;
		return apply_or_filter(orFilter);
	}
	case CARD_FILTER_NOT: {
		CardFilterNot notFilter = *filter.filter.notFilter;
		return apply_not_filter(notFilter);
	}
	case CARD_FILTER_EQUAL: {
		CardFilterEqual equalFilter = *filter.filter.equalFilter;
		return apply_equal_filter(equalFilter);
	}
	case CARD_FILTER_LIKE: {
		CardFilterLike likeFilter = *filter.filter.likeFilter;
		return apply_like_filter(likeFilter);
	}
	default:
		fprintf(stderr, "Invalid filter type %d\n", filter.type);
		exit(1);
	}
}

int
get_scryfall_ids(sqlite3 *cardDb, CardFilter filter, CStringVec *ids) {
	String sql = generate_query(filter);
	printf("%s\n", string_as_cstring(sql));
	sqlite3_stmt *stmt;
	char *err_msg = NULL;
	int error =
		sqlite3_prepare_v2(cardDb, string_as_cstring(sql), -1, &stmt, NULL);
	if (error != SQLITE_OK) {
		fprintf(stderr, "Failed to prepare cards statement: %s\n",
				sqlite3_errmsg(cardDb));
		string_free(&sql);
		sqlite3_finalize(stmt);
		return error;
	}

	*ids = cstring_vec_init();

	while ((error = sqlite3_step(stmt)) == SQLITE_ROW) {
		String cardScryfallId = string_init();
		char *cardScryfallIdStr = (char *)sqlite3_column_text(stmt, 0);
		error = string_push_str(&cardScryfallId, cardScryfallIdStr);
		if (error != 0) {
			string_free(&cardScryfallId);
			break;
		}
		cstring_vec_push(ids, string_as_cstring(cardScryfallId));
	}

	if (error != 0 && error != SQLITE_DONE) {
		fprintf(stderr, "Execution error on cards: %s\n",
				sqlite3_errmsg(cardDb));
		cstring_vec_free(ids);
		string_free(&sql);
		sqlite3_finalize(stmt);
		return error;
	}

	string_free(&sql);
	sqlite3_finalize(stmt);
	return 0;
}

CardFilter
card_filter_create_leaf(CardFilterType type, const char *key, const char *val) {
	CardFilter f = {.type = type};
	if (type == CARD_FILTER_EQUAL) {
		f.filter.equalFilter = malloc(sizeof(CardFilterEqual));
		f.filter.equalFilter->key = strdup(key);
		f.filter.equalFilter->value = strdup(val);
	} else if (type == CARD_FILTER_LIKE) {
		f.filter.likeFilter = malloc(sizeof(CardFilterLike));
		f.filter.likeFilter->key = strdup(key);
		f.filter.likeFilter->value = strdup(val);
	}
	return f;
}

CardFilter
card_filter_create_group(CardFilterType type) {
	CardFilter f = {.type = type};
	if (type == CARD_FILTER_AND) {
		f.filter.andFilter = malloc(sizeof(CardFilterAnd));
		f.filter.andFilter->filterCapacity = 4;
		f.filter.andFilter->filtersSize = 0;
		f.filter.andFilter->filters = malloc(sizeof(CardFilter) * 4);
	} else if (type == CARD_FILTER_OR) {
		f.filter.orFilter = malloc(sizeof(CardFilterOr));
		f.filter.orFilter->filterCapacity = 4;
		f.filter.orFilter->filtersSize = 0;
		f.filter.orFilter->filters = malloc(sizeof(CardFilter) * 4);
	} else if (type == CARD_FILTER_NOT) {
		f.filter.notFilter = malloc(sizeof(CardFilterNot));
		f.filter.notFilter->filter =
			card_filter_create_leaf(CARD_FILTER_LIKE, "name", "");
	}
	return f;
}

void
card_filter_free_contents(CardFilter *filter) {
	if (!filter) {
		return;
	}
	switch (filter->type) {
	case CARD_FILTER_EQUAL:
		free(filter->filter.equalFilter->key);
		free(filter->filter.equalFilter->value);
		free(filter->filter.equalFilter);
		break;
	case CARD_FILTER_LIKE:
		free(filter->filter.likeFilter->key);
		free(filter->filter.likeFilter->value);
		free(filter->filter.likeFilter);
		break;
	case CARD_FILTER_NOT:
		card_filter_free_contents(&filter->filter.notFilter->filter);
		free(filter->filter.notFilter);
		break;
	case CARD_FILTER_AND:
		for (int i = 0; i < filter->filter.andFilter->filtersSize; i++) {
			card_filter_free_contents(&filter->filter.andFilter->filters[i]);
		}
		free(filter->filter.andFilter->filters);
		free(filter->filter.andFilter);
		break;
	case CARD_FILTER_OR:
		for (int i = 0; i < filter->filter.orFilter->filtersSize; i++) {
			card_filter_free_contents(&filter->filter.orFilter->filters[i]);
		}
		free(filter->filter.orFilter->filters);
		free(filter->filter.orFilter);
		break;
	}
}

void
card_filter_group_add(CardFilter *group, CardFilter newChild) {
	CardFilter **filters = NULL;
	int *size = NULL;
	int *capacity = NULL;

	if (group->type == CARD_FILTER_AND) {
		filters = &group->filter.andFilter->filters;
		size = &group->filter.andFilter->filtersSize;
		capacity = &group->filter.andFilter->filterCapacity;
	} else if (group->type == CARD_FILTER_OR) {
		filters = &group->filter.orFilter->filters;
		size = &group->filter.orFilter->filtersSize;
		capacity = &group->filter.orFilter->filterCapacity;
	} else {
		return;
	}

	if (*size >= *capacity) {
		*capacity *= 2;
		*filters = realloc(*filters, sizeof(CardFilter) * (*capacity));
	}
	(*filters)[*size] = newChild;
	(*size)++;
}

void
card_filter_group_remove_at(CardFilter *group, int removeIdx) {
	printf("removing %d from %lx\n", removeIdx, (long int)group);
	CardFilter *arr = NULL;
	int *size = NULL;

	if (group->type == CARD_FILTER_AND) {
		arr = group->filter.andFilter->filters;
		size = &group->filter.andFilter->filtersSize;
	} else if (group->type == CARD_FILTER_OR) {
		arr = group->filter.orFilter->filters;
		size = &group->filter.orFilter->filtersSize;
	} else {
		return;
	}

	if (removeIdx < 0 || removeIdx >= *size) {
		return;
	}

	printf("removeIdx: %d from %d with size %d\n", removeIdx, group->type,
		   *size);
	card_filter_free_contents(&arr[removeIdx]);
	// Shift remaining elements left
	for (int i = removeIdx; i < *size - 1; i++) {
		printf("shifting %d to %d\n", i, i + 1);
		arr[i] = arr[i + 1];
	}
	(*size)--;
}

const char *
get_sql_column_for_key(const char *uiKey) {
	for (size_t i = 0; i < VALID_FILTER_KEYS_COUNT; i++) {
		if (strcmp(VALID_FILTER_KEYS[i].uiKey, uiKey) == 0) {
			return VALID_FILTER_KEYS[i].sqlColumn;
		}
	}
	return VALID_FILTER_KEYS[0].sqlColumn;
}