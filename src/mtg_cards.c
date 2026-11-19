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
		error = string_push_str(&query, " or ");
		if (error != 0) {
			fprintf(stderr, "Failed to alloc sql or filter 'or'\n");
			exit(1);
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
