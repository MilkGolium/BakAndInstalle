#include <windows.h>
#include <urlmon.h>
#include "sysUtil.h"

// Windows: URLDownloadToFileA downloads the file using the system's URL
// transport (WinINet).  No shell escaping needed.
int downloadFile(const char* url, const char* destPath) {
    HRESULT hr = URLDownloadToFileA(NULL, url, destPath, 0, NULL);
    return SUCCEEDED(hr) ? 1 : 0;
}
