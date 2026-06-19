#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
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
    if (strcmp(src, dst) == 0)
        return 1;  // 同路径，无需复制
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

    while (len > 1 && buf[len - 1] == '/')
        buf[--len] = '\0';

    int created = 0;

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

    int rc = mkdir(buf, 0755);
    if (rc == 0)
        created = 1;
    else if (errno != EEXIST)
        return -1;

    return created ? 1 : 0;
}

int executeScript(const char *type, const char *path) {
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
