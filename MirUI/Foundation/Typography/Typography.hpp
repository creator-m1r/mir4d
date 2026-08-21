
#pragma once

#include "Font.hpp"

namespace MirUI {

struct Typography {
    Font title;
    Font subtitle;
    Font body;
    Font caption;
    Font button;
    Font code;

    bool operator==(const Typography& other) const {
        return title == other.title &&
               subtitle == other.subtitle &&
               body == other.body &&
               caption == other.caption &&
               button == other.button &&
               code == other.code;
    }
    bool operator!=(const Typography& other) const {
        return !(*this == other);
    }

    static Typography standard() {
        Typography t;
        t.title    = {"System", 24.0, FontWeight::Bold};
        t.subtitle = {"System", 18.0, FontWeight::Medium};
        t.body     = {"System", 14.0, FontWeight::Regular};
        t.caption  = {"System", 12.0, FontWeight::Regular};
        t.button   = {"System", 14.0, FontWeight::Medium};
        t.code     = {"Menlo",   13.0, FontWeight::Regular};
        return t;
    }
};

}