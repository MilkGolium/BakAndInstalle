#ifndef DEPLOY_H
#define DEPLOY_H

#include "sqlite/sqlite3.h"

// deployAppConfig: restore config files and execute init scripts for
// a specific app platform entry (identified by app_platforms.id).
// Queries app_configs for rows matching app_platform_id:
//   - if config_path is set, copies it to deploy the config
//   - if script_path and script_type are set, executes the script
// This function is single-responsibility: it does NOT iterate over
// multiple apps, does NOT sort, and does NOT know which platform it
// is running on.  Returns 1 if all items succeed, 0 if any fail.
int deployAppConfig(sqlite3 *db, int appPlatformId);

#endif // DEPLOY_H
