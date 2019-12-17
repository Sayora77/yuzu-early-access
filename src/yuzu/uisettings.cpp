#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

// Copyright 2016 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "yuzu/uisettings.h"

namespace UISettings {

const Themes themes{{
    {"默认", "default"},
    {"黑", "qdarkstyle"},
    {"彩色", "colorful"},
    {"多彩黑暗", "colorful_dark"},
}};

Values values = {};
} // namespace UISettings
