#include "sqlite3.h"
#include "recomp_api.h"

#define DB_FILE "ProxyMM_KV.db"

sqlite3 *db;

RECOMP_EXPORT int KV_Init() {
    if (sqlite3_open(DB_FILE, &db) != SQLITE_OK) {
        recomp_printf("Can't open database: %s", sqlite3_errmsg(db));
        return 0;
    }
    const char *sql = "CREATE TABLE IF NOT EXISTS storage (key TEXT PRIMARY KEY, value BLOB NOT NULL);";
    return sqlite3_exec(db, sql, 0, 0, 0) == SQLITE_OK;
}

RECOMP_EXPORT void KV_Teardown() {
    sqlite3_close(db);
}

RECOMP_EXPORT int KV_SetItem(const char *key, const void *data, size_t size) {
    const char *sql = "INSERT INTO storage (key, value) VALUES (?, ?) ON CONFLICT(key) DO UPDATE SET value = excluded.value;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) return 0;
    sqlite3_bind_text(stmt, 1, key, -1, SQLITE_STATIC);
    sqlite3_bind_blob(stmt, 2, data, size, SQLITE_STATIC);
    int res = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return res;
}

RECOMP_EXPORT int KV_GetItem(const char *key, void *dest, size_t expected_size) {
    const char *sql = "SELECT value FROM storage WHERE key = ?;";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) return 0;
    sqlite3_bind_text(stmt, 1, key, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        size_t stored_size = sqlite3_column_bytes(stmt, 0);
        if (stored_size != expected_size) {  // Fail if sizes don't match
            sqlite3_finalize(stmt);
            return 0;
        }
        Lib_MemCpy(dest, sqlite3_column_blob(stmt, 0), stored_size);
        sqlite3_finalize(stmt);
        return 1;
    }

    sqlite3_finalize(stmt);
    return 0;
}

RECOMP_EXPORT int KV_RemoveItem(const char *key) {
    const char *sql = "DELETE FROM storage WHERE key = ?;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) return 0;
    sqlite3_bind_text(stmt, 1, key, -1, SQLITE_STATIC);
    int res = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return res;
}

RECOMP_EXPORT int KV_Clear() {
    return sqlite3_exec(db, "DELETE FROM storage;", 0, 0, 0) == SQLITE_OK;
}

RECOMP_CALLBACK("*", recomp_on_play_init)
void OnPlayInit(PlayState* play) {
    KV_Init();

    KV_SetItem("key", "value", 6);
    char buffer[6];
    KV_GetItem("key", buffer, sizeof(buffer));

    recomp_printf("result: %s", buffer[0]);

    KV_Teardown();
}
