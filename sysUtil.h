#ifndef SYSUTIL_H
#define SYSUTIL_H

#include "sqlite/sqlite3.h"

// downloadFile: download a file from url and save it to destPath.
// Returns 1 on success, 0 on failure.
int downloadFile(const char* url, const char* destPath);

// copyPath: recursively copy src (file or directory) to dst.
// Returns 1 on success, 0 on failure.
int copyPath(const char* src, const char* dst);

// removePath: delete a file (isDir=0) or a directory tree (isDir=1).
// Returns 1 on success, 0 on failure.
int removePath(const char* path, int isDir);

// createDir: create directories recursively (mkdir -p equivalent).
//   Returns  1 if directories were created.
//   Returns  0 if the target already exists.
//   Returns -1 on error (permission, disk full, quota, etc.).
int createDir(const char* path);

// deployAppConfig: restore config files and execute init scripts for
// a specific app platform entry (identified by app_platforms.id).
// Queries app_configs for rows matching app_platform_id:
//   - if config_path is set, copies it to deploy the config
//   - if script_path and script_type are set, executes the script
// This function is single-responsibility: it does NOT iterate over
// multiple apps, does NOT sort, and does NOT know which platform it
// is running on.  Returns 1 if all items succeed, 0 if any fail.
int deployAppConfig(sqlite3 *db, int appPlatformId);

#endif // SYSUTIL_H
