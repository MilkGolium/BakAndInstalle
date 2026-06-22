#include <stdlib.h>
#include <string.h>
#include "tui.h"

// 全局单例 —— View 只读，Controller 读写
TuiState g_tui;

void tuiInitState(void) {
    memset(&g_tui, 0, sizeof(g_tui));

    // 默认焦点在分类列表
    g_tui.focus = TUI_FOCUS_CATEGORIES;
    g_tui.view  = TUI_VIEW_BROWSE;

    // -1 表示"未选中"
    g_tui.selectedCategoryId  = -1;
    g_tui.selectedAppIndex    = -1;
    g_tui.selectedConfigIndex = -1;
    g_tui.dialogResult        = -1;

    // 默认平台：检测宿主 OS
#if defined(_WIN32) || defined(_WIN64)
    strncpy(g_tui.currentPlatform, "Windows", sizeof(g_tui.currentPlatform) - 1);
#elif defined(__APPLE__)
    strncpy(g_tui.currentPlatform, "macOS", sizeof(g_tui.currentPlatform) - 1);
#elif defined(__linux__)
    strncpy(g_tui.currentPlatform, "Linux", sizeof(g_tui.currentPlatform) - 1);
#else
    // 其他（FreeBSD 等）留空表示全部
    g_tui.currentPlatform[0] = '\0';
#endif
}

void tuiFreeState(void) {
    free(g_tui.categories);
    free(g_tui.apps);
    free(g_tui.configs);

    // 清零防野指针
    memset(&g_tui, 0, sizeof(g_tui));
}
