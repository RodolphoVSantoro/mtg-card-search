#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "sqlite3.h"
#include "mtg_filter.h"

#include "collections.h"

int get_cards(sqlite3 *imageDb) {
	int error;
	const char *baseSql = "select"
        "ci.scryfallId"
        "from cards ca"
        "join cardIdentifiers ci on ci.uuid = ca.uuid";
	sqlite3_stmt *stmt;
	char *err_msg = NULL;
	error = sqlite3_prepare_v2(imageDb, baseSql, -1, &stmt, NULL);
	if (error != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(imageDb));
		sqlite3_close(imageDb);
		return 1;
	}

	uint8_t *blob_data_sdl = NULL;
	int blob_size_sdl;
	while ((error = sqlite3_step(stmt)) == SQLITE_ROW) {
        const char *fileName = (const char *)sqlite3_column_text(stmt, 0);
        const void *blob_data = sqlite3_column_blob(stmt, 1);
        int blob_size = sqlite3_column_bytes(stmt, 1);
        if(blob_data_sdl == NULL) {
	        blob_data_sdl = (uint8_t*) malloc(blob_size);
	        blob_size_sdl = blob_size;
	        memcpy(blob_data_sdl, blob_data, blob_size);
	    }
        printf("File name: %s | img Size: %d bytes\n", fileName, blob_size);
    }

    if (error != SQLITE_DONE) {
        fprintf(stderr, "Execution error: %s\n", sqlite3_errmsg(imageDb));
        return 1;
    }
    sqlite3_finalize(stmt);
}
