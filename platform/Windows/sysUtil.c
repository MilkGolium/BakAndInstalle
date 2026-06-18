#include <windows.h>
#include <urlmon.h>
#include <shellapi.h>
#include <stdio.h>
#include <string.h>
#include "sqlite/sqlite3.h"
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

int removePath(const char* path, int isDir) {
    if (isDir) {
        // SHFileOperation with FO_DELETE for recursive directory deletion
        char buf[1024] = {0};
        strncpy(buf, path, sizeof(buf) - 2);

        SHFILEOPSTRUCTA op = {0};
        op.wFunc = FO_DELETE;
        op.pFrom = buf;
        op.fFlags = FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT;

        return SHFileOperationA(&op) == 0 ? 1 : 0;
    }

    return DeleteFileA(path) ? 1 : 0;
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

// -- deployAppConfig 辅助函数 --

static int executeScript(const char *type, const char *path) {
    if (strcmp(type, "Bat") == 0) {
        // 直接执行批处理文件
        size_t len = strlen(path);
        char *cmd = (char*)malloc(len + 4);
        if (!cmd) return 0;
        cmd[0] = '"';
        memcpy(cmd + 1, path, len);
        cmd[len + 1] = '"';
        cmd[len + 2] = '\0';
        int ret = system(cmd);
        free(cmd);
        return ret == 0 ? 1 : 0;
    }
    if (strcmp(type, "PowerShell") == 0) {
        // powershell -ExecutionPolicy Bypass -File "path"
        size_t len = strlen(path);
        char *cmd = (char*)malloc(len + 48);
        if (!cmd) return 0;
        memcpy(cmd, "powershell -ExecutionPolicy Bypass -File \"", 42);
        memcpy(cmd + 42, path, len);
        cmd[len + 42] = '"';
        cmd[len + 43] = '\0';
        int ret = system(cmd);
        free(cmd);
        return ret == 0 ? 1 : 0;
    }
    fprintf(stderr, "executeScript: unsupported type '%s' for path %s\n",
            type, path);
    return 0;
}

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

        // 恢复配置文件
        if (configPath && configPath[0]) {
            if (!copyPath(configPath, configPath)) {
                // copyPath 可能因源目路径相同而返回失败；
                // 此时检查配置文件是否已就位
                if (GetFileAttributesA(configPath) == INVALID_FILE_ATTRIBUTES) {
                    fprintf(stderr,
                        "deployAppConfig: config not found: %s\n",
                        configPath);
                    ok = 0;
                }
            }
        }

        // 执行脚本
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
