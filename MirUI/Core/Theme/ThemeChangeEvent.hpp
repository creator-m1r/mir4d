
#pragma once

#include "ThemeID.hpp"

namespace MirUI {

struct ThemeChangeEvent {
    ThemeID oldThemeId;
    ThemeID newThemeId;
};

}