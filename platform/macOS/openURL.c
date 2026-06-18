#include <stdlib.h>
#include <string.h>
#include "openURL.h"

// macOS: use the 'open' command-line tool
int openURL(const char* url) {
    size_t len = strlen(url);
    // Each char may need a backslash prefix, plus the fixed parts
    char* cmd = (char*)malloc(len * 2 + 16);
    if (!cmd) return 0;

    char* p = cmd;
    *p++ = 'o'; *p++ = 'p'; *p++ = 'e'; *p++ = 'n'; *p++ = ' '; *p++ = '"';
    for (size_t i = 0; i < len; i++) {
        if (url[i] == '"' || url[i] == '\\')
            *p++ = '\\';
        *p++ = url[i];
    }
    *p++ = '"';
    *p = '\0';

    int ret = system(cmd);
    free(cmd);
    return ret == 0 ? 1 : 0;
}
