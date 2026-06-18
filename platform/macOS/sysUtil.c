#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include "sqlite/sqlite3.h"
#include "sysUtil.h"

// Escape " and \ for safe shell quoting
static void escape(char *out, const char *s, size_t *pos) {
    while (*s) {
        if (*s == '"' || *s == '\\')
            out[(*pos)++] = '\\';
        out[(*pos)++] = *s++;
    }
}

int downloadFile(const char* url, const char* destPath) {
    // Build: curl -L -o "destPath" "url"
    size_t ulen = strlen(url);
    size_t dlen = strlen(destPath);
    // Worst case: every char in url/destPath is " or \, doubling length.
    // Format: curl -L -o "dst" "url" + NUL
    //   = 12 + dst_len*2 + 3 + url_len*2 + 1 + 1 = url_len*2 + dst_len*2 + 17
    char *cmd = (char*)malloc(ulen * 2 + dlen * 2 + 17);
    if (!cmd) return 0;

    size_t p = 0;
    memcpy(cmd + p, "curl -L -o \"", 12); p += 12;
    escape(cmd, destPath, &p);
    cmd[p++] = '"';
    cmd[p++] = ' ';
    cmd[p++] = '"';
    escape(cmd, url, &p);
    cmd[p++] = '"';
    cmd[p]   = '\0';

    int ret = system(cmd);
    free(cmd);
    return ret == 0 ? 1 : 0;
}

int copyPath(const char* src, const char* dst) {
    // Build: cp -R "src" "dst"
    size_t slen = strlen(src);
    size_t dlen = strlen(dst);
    // Worst case: src/dst chars double via escape.
    // Format: cp -R "src" "dst" + NUL
    //   = 7 + slen*2 + 3 + dlen*2 + 1 + 1 = slen*2 + dlen*2 + 12
    char *cmd = (char*)malloc(slen * 2 + dlen * 2 + 12);
    if (!cmd) return 0;

    size_t p = 0;
    memcpy(cmd + p, "cp -R \"", 7); p += 7;
    escape(cmd, src, &p);
    cmd[p++] = '"';
    cmd[p++] = ' ';
    cmd[p++] = '"';
    escape(cmd, dst, &p);
    cmd[p++] = '"';
    cmd[p]   = '\0';

    int ret = system(cmd);
    free(cmd);
    return ret == 0 ? 1 : 0;
}

int removePath(const char* path, int isDir) {
    if (access(path, F_OK) != 0)
        return 0;  // does not exist

    if (isDir) {
        size_t len = strlen(path);
        char *cmd = (char*)malloc(len * 2 + 16);
        if (!cmd) return 0;

        size_t p = 0;
        memcpy(cmd + p, "rm -rf \"", 8); p += 8;
        escape(cmd, path, &p);
        cmd[p++] = '"';
        cmd[p]   = '\0';

        int ret = system(cmd);
        free(cmd);
        return ret == 0 ? 1 : 0;
    }

    return unlink(path) == 0 ? 1 : 0;
}

int createDir(const char* path) {
    size_t len = strlen(path);
    if (len == 0 || len >= 4096) return -1;

    char buf[4096];
    memcpy(buf, path, len + 1);

    // Strip trailing slash (preserve root "/")
    while (len > 1 && buf[len - 1] == '/')
        buf[--len] = '\0';

    int created = 0;

    // Walk through each path component
    for (char *p = buf + 1; *p; p++) {
        if (*p != '/') continue;
        *p = '\0';

        int rc = mkdir(buf, 0755);
        if (rc == 0)
            created = 1;
        else if (errno != EEXIST)
            return -1;

        *p = '/';
    }

    // Final component
    int rc = mkdir(buf, 0755);
    if (rc == 0)
        created = 1;
    else if (errno != EEXIST)
        return -1;

    return created ? 1 : 0;
}

// -- deployAppConfig 辅助函数 --

static int executeScript(const char *type, const char *path) {
    if (strcmp(type, "Shell") != 0 && strcmp(type, "sh") != 0) {
        fprintf(stderr, "executeScript: unsupported type '%s' for path %s\n",
                type, path);
        return 0;
    }
    // Build: sh "path"
    // Build: sh "escaped_path" + NUL  =>  4 + len*2 + 1 + 1 = len*2 + 6
    size_t len = strlen(path);
    char *cmd = (char*)malloc(len * 2 + 6);
    if (!cmd) return 0;

    size_t p = 0;
    memcpy(cmd + p, "sh \"", 4); p += 4;
    escape(cmd, path, &p);
    cmd[p++] = '"';
    cmd[p]   = '\0';

    int ret = system(cmd);
    free(cmd);
    return ret == 0 ? 1 : 0;
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
                if (access(configPath, F_OK) != 0) {
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
