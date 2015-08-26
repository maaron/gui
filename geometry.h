#pragma once

#include <d2d1.h>

#undef min

namespace drawing
{
    typedef FLOAT distance;
    
    struct rectangle : D2D_RECT_F
    {
        rectangle(distance left, distance top, distance right, distance bottom)
            : D2D_RECT_F({ left, top, right, bottom }) {}

        rectangle() : D2D_RECT_F({ 0, 0, 0, 0 }) {}

        distance height() const { return bottom - top; }
        distance width() const { return right - left; }
    };

    struct point : D2D_POINT_2F
    {
        point(distance x, distance y)
            : D2D_POINT_2F({ x, y }) {}

        point() : D2D_POINT_2F({ 0, 0 }) {}
    };

    typedef D2D1_COLOR_F color;

    bool empty(rectangle const& r)
    {
        return r.top == r.bottom ||
            r.left == r.right;
    }

    rectangle from_top(rectangle const& r, distance d)
    {
        return rectangle(
            r.left,
            r.top,
            r.right,
            std::min(r.top + d, r.bottom));
    }

    rectangle below(rectangle const& r, distance d)
    {
        return rectangle(
            r.left,
            std::min(r.top + d, r.bottom),
            r.right,
            r.bottom);
    }

    rectangle below(rectangle const& r, rectangle const& d)
    {
        return rectangle(
            r.left,
            std::min(d.bottom, r.bottom),
            r.right,
            r.bottom);
    }

    rectangle from_left(rectangle const& r, distance d)
    {
        return rectangle(
            r.left,
            r.top,
            std::min(r.left + d, r.right),
            r.bottom);
    }

    rectangle right_of(rectangle const& r, distance d)
    {
        return rectangle(
            std::min(r.left + d, r.right),
            r.top,
            r.right,
            r.bottom);
    }

    rectangle right_of(rectangle const& r, rectangle const& d)
    {
        return rectangle(
            std::min(d.right, r.right),
            r.top,
            r.right,
            r.bottom);
    }

    rectangle inside(rectangle const& r, distance d)
    {
        return rectangle(
            r.left + d, r.top + d,
            r.right - d, r.bottom - d);
    }
}