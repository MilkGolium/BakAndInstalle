#ifndef SYSUTIL_H
#define SYSUTIL_H

// downloadFile: download a file from url and save it to destPath.
// Returns 1 on success, 0 on failure.
int downloadFile(const char* url, const char* destPath);

// copyPath: recursively copy src (file or directory) to dst.
// Returns 1 on success, 0 on failure.
int copyPath(const char* src, const char* dst);

// createDir: create directories recursively (mkdir -p equivalent).
//   Returns  1 if directories were created.
//   Returns  0 if the target already exists.
//   Returns -1 on error (permission, disk full, quota, etc.).
int createDir(const char* path);

#endif // SYSUTIL_H
