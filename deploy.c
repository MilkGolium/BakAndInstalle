#include <stdio.h>
#include "sqlite/sqlite3.h"
#include "sysUtil.h"
#include "deploy.h"

int deployAppConfig(sqlite3 *db, int appPlatformId) {
    static const char *sql =
        "SELECT config_path, script_type, script_path "
        "FROM app_configs "
        "WHERE app_platform_id = ?1;";

    sqlite3_stmt *stmt = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "deployAppConfig: prepare failed: %s\n",
                sqlite3_errmsg(db));
        return 0;
    }

    sqlite3_bind_int(stmt, 1, appPlatformId);

    int ok = 1;
    int rc;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        const char *configPath = (const char *)sqlite3_column_text(stmt, 0);
        const char *scriptType = (const char *)sqlite3_column_text(stmt, 1);
        const char *scriptPath = (const char *)sqlite3_column_text(stmt, 2);

        if (configPath && configPath[0]) {
            if (!copyPath(configPath, configPath)) {
                fprintf(stderr,
                    "deployAppConfig: config not found: %s\n",
                    configPath);
                ok = 0;
            }
        }

        if (scriptType && scriptType[0] && scriptPath && scriptPath[0]) {
            if (!executeScript(scriptType, scriptPath)) {
                fprintf(stderr,
                    "deployAppConfig: script failed: %s (%s)\n",
                    scriptPath, scriptType);
                ok = 0;
            }
        }
    }

    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "deployAppConfig: step failed: %s\n",
                sqlite3_errmsg(db));
        return 0;
    }

    return ok;
}
