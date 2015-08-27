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

        target(target const& t, rectangle const& r)
            : rtarget(t.rtarget), bounds(r) {}
    };

    bool empty(target const& t)
    {
        return empty(t.bounds);
    }

    target from_top(const target& t, distance d)
    {
        return target(t, from_top(t.bounds, d));
    }

    target below(const target& t, distance d)
    {
        return target(t, below(t.bounds, d));
    }

    target below(const target& t, rectangle const& d)
    {
        return target(t, below(t.bounds, d));
    }

    target above(const target& t, distance d)
    {
        return target(t, above(t.bounds, d));
    }

    target above(const target& t, rectangle const& d)
    {
        return target(t, above(t.bounds, d));
    }

    target from_left(const target& t, distance d)
    {
        return target(t, from_left(t.bounds, d));
    }

    target right_of(const target& t, distance d)
    {
        return target(t, right_of(t.bounds, d));
    }

    target right_of(const target& t, rectangle const& d)
    {
        return target(t, right_of(t.bounds, d));
    }

    target inside(const target& t, distance d)
    {
        return target(t, inside(t.bounds, d));
    }

    target centered(target const& t, point const& size)
    {
        return target(t, centered(t.bounds, size));
    }
}