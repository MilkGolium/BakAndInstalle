#include <windows.h>
#include <urlmon.h>
#include <shellapi.h>
#include <string.h>
#include "sysUtil.h"

// Windows: URLDownloadToFileA downloads the file using the system's URL
// transport (WinINet).  No shell escaping needed.
int downloadFile(const char* url, const char* destPath) {
    HRESULT hr = URLDownloadToFileA(NULL, url, destPath, 0, NULL);
    return SUCCEEDED(hr) ? 1 : 0;
}

// Windows: SHFileOperation copies files/directories recursively.
// pFrom and pTo must be double-null-terminated strings.
int copyPath(const char* src, const char* dst) {
    char srcBuf[1024] = {0};
    char dstBuf[1024] = {0};

    strncpy(srcBuf, src, sizeof(srcBuf) - 2);
    strncpy(dstBuf, dst, sizeof(dstBuf) - 2);

    SHFILEOPSTRUCTA op = {0};
    op.wFunc = FO_COPY;
    op.pFrom = srcBuf;
    op.pTo   = dstBuf;
    op.fFlags = FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT;

    return SHFileOperationA(&op) == 0 ? 1 : 0;
}

int createDir(const char* path) {
    size_t len = strlen(path);
    if (len == 0 || len >= 4096) return -1;

    char buf[4096];
    memcpy(buf, path, len + 1);

    // Normalise forward slashes to backslashes
    for (char *p = buf; *p; p++)
        if (*p == '/') *p = '\\';

    // Strip trailing backslash (preserve root "C:\")
    while (len > 1 && buf[len - 1] == '\\')
        buf[--len] = '\0';

    int created = 0;

    // Skip past "C:\" or "\\server\share\" so we don't try to create root
    char *start = buf;
    if (buf[0] && buf[1] == ':')         // "C:\..."
        start = buf + 2;
    else if (buf[0] == '\\' && buf[1] == '\\') { // "\\server\share\..."
        start = buf + 2;
        while (*start && *start != '\\') start++; // skip server
        if (*start) start++;                       // skip backslash
        while (*start && *start != '\\') start++; // skip share
        if (*start) start++;                       // skip backslash
    } else if (buf[0] == '\\') {
        start = buf + 1;  // "\foo" relative to current drive root
    }

    // Walk each component and try to create it
    for (char *p = start; *p; p++) {
        if (*p != '\\') continue;
        *p = '\0';

        if (CreateDirectoryA(buf, NULL))
            created = 1;
        else if (GetLastError() != ERROR_ALREADY_EXISTS)
            return -1;

        *p = '\\';
    }

    // Final component
    if (CreateDirectoryA(buf, NULL))
        created = 1;
    else if (GetLastError() != ERROR_ALREADY_EXISTS)
        return -1;

    return created ? 1 : 0;
}
