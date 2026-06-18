#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "openURL.h"

// Linux / other Unix: use xdg-open
int openURL(const char* url) {
    size_t len = strlen(url);
    char* cmd = (char*)malloc(len * 2 + 24);
    if (!cmd) return 0;

    char* p = cmd;
    p += sprintf(p, "xdg-open \"");
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
