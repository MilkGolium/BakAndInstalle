#ifndef SYSUTIL_H
#define SYSUTIL_H

// downloadFile: download a file from url and save it to destPath.
// Returns 1 on success, 0 on failure.
int downloadFile(const char* url, const char* destPath);

#endif // SYSUTIL_H
