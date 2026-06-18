#include <stdio.h>
#include <string.h>
#include "sqlite/sqlite3.h"
#include "dbAPI.h"

int addCategory(sqlite3 *db, const char *name) {
    sqlite3_stmt *stmt = NULL;
    const char *sql = "INSERT INTO categories (name) VALUES (?1);";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "addCategory: prepare failed: %s\n", sqlite3_errmsg(db));
        return 0;
    }

    sqlite3_bind_text(stmt, 1, name, -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "addCategory: %s\n", sqlite3_errmsg(db));
        return 0;
    }

    return 1;
}

int removeCategory(sqlite3 *db, int id) {
    sqlite3_stmt *stmt = NULL;
    const char *sql = "DELETE FROM categories WHERE id = ?1;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "removeCategory: prepare failed: %s\n", sqlite3_errmsg(db));
        return 0;
    }

    sqlite3_bind_int(stmt, 1, id);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "removeCategory: %s\n", sqlite3_errmsg(db));
        return 0;
    }

    return 1;
}

int addApp(sqlite3 *db, const char *name, int category_id,
           int priority, const char *description) {
    sqlite3_stmt *stmt = NULL;
    const char *sql = "INSERT INTO apps (name, category_id, priority, description) "
                      "VALUES (?1, ?2, ?3, ?4);";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "addApp: prepare failed: %s\n", sqlite3_errmsg(db));
        return 0;
    }

    sqlite3_bind_text(stmt, 1, name, -1, SQLITE_TRANSIENT);

    if (category_id <= 0)
        sqlite3_bind_null(stmt, 2);
    else
        sqlite3_bind_int(stmt, 2, category_id);

    sqlite3_bind_int(stmt, 3, priority);

    if (description == NULL || description[0] == '\0')
        sqlite3_bind_null(stmt, 4);
    else
        sqlite3_bind_text(stmt, 4, description, -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "addApp: %s\n", sqlite3_errmsg(db));
        return 0;
    }

    return 1;
}

int removeApp(sqlite3 *db, int id) {
    sqlite3_stmt *stmt = NULL;
    const char *sql = "DELETE FROM apps WHERE id = ?1;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "removeApp: prepare failed: %s\n", sqlite3_errmsg(db));
        return 0;
    }

    sqlite3_bind_int(stmt, 1, id);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "removeApp: %s\n", sqlite3_errmsg(db));
        return 0;
    }

    return 1;
}

int addAppPlatform(sqlite3 *db, int appId, const char *platform,
                   int isManual, const char *downloadUrl,
                   const char *installerPath)
{
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "INSERT OR IGNORE INTO app_platforms "
        "(app_id, platform, is_manual, download_url, installer_path) "
        "VALUES (?1, ?2, ?3, ?4, ?5);";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "addAppPlatform: prepare failed: %s\n",
                sqlite3_errmsg(db));
        return 0;
    }

    sqlite3_bind_int(stmt, 1, appId);
    sqlite3_bind_text(stmt, 2, platform, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, isManual ? 1 : 0);

    if (downloadUrl && downloadUrl[0])
        sqlite3_bind_text(stmt, 4, downloadUrl, -1, SQLITE_TRANSIENT);
    else
        sqlite3_bind_null(stmt, 4);

    if (installerPath && installerPath[0])
        sqlite3_bind_text(stmt, 5, installerPath, -1, SQLITE_TRANSIENT);
    else
        sqlite3_bind_null(stmt, 5);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "addAppPlatform: %s\n", sqlite3_errmsg(db));
        return 0;
    }

    // sqlite3_changes == 0 means UNIQUE constraint suppressed the insert
    if (sqlite3_changes(db) == 0)
        return 0;

    return 1;
}

int removeAppPlatform(sqlite3 *db, int id) {
    sqlite3_stmt *stmt = NULL;
    const char *sql = "DELETE FROM app_platforms WHERE id = ?1;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "removeAppPlatform: prepare failed: %s\n",
                sqlite3_errmsg(db));
        return 0;
    }

    sqlite3_bind_int(stmt, 1, id);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "removeAppPlatform: %s\n", sqlite3_errmsg(db));
        return 0;
    }

    return 1;
}

int addAppConfig(sqlite3 *db, int appPlatformId,
                 const char *configPath, const char *scriptType,
                 const char *scriptPath)
{
    sqlite3_stmt *stmt = NULL;
    const char *sql =
        "INSERT INTO app_configs "
        "(app_platform_id, config_path, script_type, script_path) "
        "VALUES (?1, ?2, ?3, ?4);";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "addAppConfig: prepare failed: %s\n",
                sqlite3_errmsg(db));
        return 0;
    }

    sqlite3_bind_int(stmt, 1, appPlatformId);

    if (configPath && configPath[0])
        sqlite3_bind_text(stmt, 2, configPath, -1, SQLITE_TRANSIENT);
    else
        sqlite3_bind_null(stmt, 2);

    if (scriptType && scriptType[0])
        sqlite3_bind_text(stmt, 3, scriptType, -1, SQLITE_TRANSIENT);
    else
        sqlite3_bind_null(stmt, 3);

    if (scriptPath && scriptPath[0])
        sqlite3_bind_text(stmt, 4, scriptPath, -1, SQLITE_TRANSIENT);
    else
        sqlite3_bind_null(stmt, 4);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "addAppConfig: %s\n", sqlite3_errmsg(db));
        return 0;
    }

    return 1;
}

int removeAppConfig(sqlite3 *db, int id) {
    sqlite3_stmt *stmt = NULL;
    const char *sql = "DELETE FROM app_configs WHERE id = ?1;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "removeAppConfig: prepare failed: %s\n",
                sqlite3_errmsg(db));
        return 0;
    }

    sqlite3_bind_int(stmt, 1, id);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "removeAppConfig: %s\n", sqlite3_errmsg(db));
        return 0;
    }

    return 1;
}

int queryAppsByPlatform(sqlite3 *db, const char *platform,
    int (*callback)(void *context, const AppInfo *app), void *context)
{
    static const char *sql =
        "SELECT a.id, a.name, a.category_id, a.priority, a.description "
        "FROM apps a "
        "JOIN app_platforms ap ON ap.app_id = a.id "
        "WHERE ap.platform = ?1 "
        "ORDER BY a.priority ASC;";

    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "queryAppsByPlatform: prepare failed: %s\n",
                sqlite3_errmsg(db));
        return 0;
    }

    sqlite3_bind_text(stmt, 1, platform, -1, SQLITE_TRANSIENT);

    int rc;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        AppInfo app;

        app.id          = sqlite3_column_int(stmt, 0);
        app.category_id = sqlite3_column_int(stmt, 2);
        app.priority    = sqlite3_column_int(stmt, 3);

        snprintf(app.name, sizeof(app.name), "%s",
                 (const char *)sqlite3_column_text(stmt, 1));

        const unsigned char *desc = sqlite3_column_text(stmt, 4);
        if (desc)
            snprintf(app.description, sizeof(app.description), "%s",
                     (const char *)desc);
        else
            app.description[0] = '\0';

        if (callback && callback(context, &app) != 0)
            break;
    }

    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE && rc != SQLITE_ROW) {
        fprintf(stderr, "queryAppsByPlatform: step failed: %s\n",
                sqlite3_errmsg(db));
        return 0;
    }

    return 1;
}
