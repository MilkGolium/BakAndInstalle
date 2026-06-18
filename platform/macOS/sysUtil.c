#include <stdlib.h>
#include <string.h>
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
    // Prefix: "curl -L -o \0" = 12 chars, plus 2 escaped quotes = +4
    char *cmd = (char*)malloc(ulen + dlen + 20);
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
