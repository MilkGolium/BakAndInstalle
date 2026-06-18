#ifndef DBAPI_H
#define DBAPI_H

#include "sqlite/sqlite3.h"

// addCategory: insert a new category. Returns 1 on success, 0 on failure.
int addCategory(sqlite3 *db, const char *name);

// removeCategory: delete a category by id (apps get category_id set to NULL).
// Returns 1 on success, 0 on failure.
int removeCategory(sqlite3 *db, int id);

// addApp: insert a new software entry. category_id <= 0 means uncategorised.
// priority: lower = earlier install order. description may be NULL.
// Returns 1 on success, 0 on failure.
int addApp(sqlite3 *db, const char *name, int category_id,
           int priority, const char *description);

// removeApp: delete an app by id.  Associated app_platforms and app_configs
// are cleaned up automatically via ON DELETE CASCADE.
// Returns 1 on success, 0 on failure.
int removeApp(sqlite3 *db, int id);

// addAppPlatform: bind an app to a platform.  Returns 1 on success,
// 0 if the record already exists (UNIQUE violation) or on error.
int addAppPlatform(sqlite3 *db, int appId, const char *platform,
                   int isManual, const char *downloadUrl,
                   const char *installerPath);

// removeAppPlatform: delete a platform record by id.  Associated
// app_configs are cleaned up via ON DELETE CASCADE.
// Returns 1 on success, 0 on failure.
int removeAppPlatform(sqlite3 *db, int id);

// addAppConfig: add a config/script entry under a platform record.
// All text parameters may be NULL or empty.  Returns 1 on success, 0 on failure.
int addAppConfig(sqlite3 *db, int appPlatformId,
                 const char *configPath, const char *scriptType,
                 const char *scriptPath);

// removeAppConfig: delete a config record by id.  Returns 1 on success, 0 on failure.
int removeAppConfig(sqlite3 *db, int id);

// AppInfo: single result row returned via callback by queryAppsByPlatform.
// Strings are copied into fixed-size buffers so they are valid throughout
// the callback invocation.
typedef struct {
    int id;
    int category_id;
    int priority;
    char name[256];
    char description[512];
} AppInfo;

// queryAppsByPlatform: query all apps that have a platform entry for the
// given platform, sorted by priority ascending (lowest first).
//
// callback(context, &app) is invoked once per result row.  The callback
// should return 0 to continue iteration or non-zero to abort early.
// Returns 1 if the query completed (possibly aborted by callback),
// 0 on SQL error.
int queryAppsByPlatform(sqlite3 *db, const char *platform,
    int (*callback)(void *context, const AppInfo *app), void *context);

#endif // DBAPI_H
