#ifndef SYSUTIL_H
#define SYSUTIL_H

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

// executeScript: execute a script file identified by type ("Shell", "Bat",
// "PowerShell", etc.) and path.  Implemented per platform in sysUtil.c.
// Returns 1 on success, 0 on failure.
int executeScript(const char *type, const char *path);

#endif // SYSUTIL_H
