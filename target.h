#pragma once

#include <d2d1.h>

#include "geometry.h"
#include "drawing.h"

namespace drawing
{
    struct target : rectangle
    {
        d2d::render_target* rtarget;

        target() : rtarget(nullptr), rectangle(0, 0, 0, 0)
        {}

        target(d2d::render_target* rt) : rtarget(rt)
        {
            auto size = rt->get_target()->GetSize();
            top = 0;
            left = 0;
            bottom = size.height;
            right = size.width;
        }

        target(target const& t, rectangle const& r)
            : rtarget(t.rtarget), rectangle(r) {}

        rectangle& bounds() { return *this; }
        rectangle const& bounds() const { return *this; }
    };

    target from_top(const target& t, distance d)
    {
        return target(t, from_top(t.bounds(), d));
    }

    target to_top(const target& t, distance d)
    {
        return target(t, to_top(t.bounds(), d));
    }

    target below(const target& t, distance d)
    {
        return target(t, below(t.bounds(), d));
    }

    target below(const target& t, rectangle const& d)
    {
        return target(t, below(t.bounds(), d));
    }

    target above(const target& t, distance d)
    {
        return target(t, above(t.bounds(), d));
    }

    target above(const target& t, rectangle const& d)
    {
        return target(t, above(t.bounds(), d));
    }

    target from_left(const target& t, distance d)
    {
        return target(t, from_left(t.bounds(), d));
    }

    target to_right(const target& t, distance d)
    {
        return target(t, to_right(t.bounds(), d));
    }

    target to_right(const target& t, rectangle const& d)
    {
        return target(t, to_right(t.bounds(), d));
    }

    target inside(const target& t, distance d)
    {
        return target(t, inside(t.bounds(), d));
    }

    target centered(target const& t, point const& size)
    {
        return target(t, centered(t.bounds(), size));
    }
}