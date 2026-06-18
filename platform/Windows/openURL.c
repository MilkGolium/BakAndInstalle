#include <windows.h>
#include "openURL.h"

// Windows: ShellExecute opens the URL with the default handler
int openURL(const char* url) {
    HINSTANCE ret = ShellExecuteA(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL);
    return ((INT_PTR)ret) > 32 ? 1 : 0;
}
