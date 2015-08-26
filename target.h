#pragma once

#include <d2d1.h>

#include "geometry.h"
#include "drawing.h"

namespace drawing
{
    struct target
    {
        d2d::render_target* rtarget;
        rectangle bounds;

        target() : rtarget(nullptr), bounds({ 0, 0, 0, 0 })
        {}

        target(d2d::render_target* rt) : rtarget(rt)
        {
            auto size = rt->get_target()->GetSize();
            bounds.top = 0;
            bounds.left = 0;
            bounds.bottom = size.height;
            bounds.right = size.width;
        }
    };
}