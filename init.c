// 此文件存放初始化代码，包括数据库初始化函数

#include <stdio.h>
#include "sqlite/sqlite3.h"
#include "init.h"

// initDatabase: open (or create) the SQLite database at dbPath and execute
// the given SQL script.  Returns 1 on success, 0 on failure and prints
// the error message to stderr.
int initDatabase(const char* dbPath, const char* sql) {
    sqlite3* db = NULL;
    char* errMsg = NULL;

    // Open / create the database
    if (sqlite3_open(dbPath, &db) != SQLITE_OK) {
        fprintf(stderr, "sqlite3_open failed: %s\n", sqlite3_errmsg(db));
        if (db) sqlite3_close(db);
        return 0;
    }

    // PRAGMA foreign_keys 必须在每个连接建立后立即单独执行，
    // 绝不能和 CREATE TABLE 揉在同一个 sqlite3_exec 批处理中，
    // 否则可能因事务/批处理机制被静默忽略。
    sqlite3_exec(db, "PRAGMA foreign_keys = ON;", NULL, NULL, NULL);

    // Execute the SQL script
    int rc = sqlite3_exec(db, sql, NULL, NULL, &errMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "sqlite3_exec failed: %s\n", errMsg);
        sqlite3_free(errMsg);
        sqlite3_close(db);
        return 0;
    }

    sqlite3_close(db);
    return 1;
}
