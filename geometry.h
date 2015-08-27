#pragma once

#include <d2d1.h>

#undef min

namespace drawing
{
    typedef FLOAT distance;

    struct point;
    struct line;
    struct rectangle;
    
    struct point : D2D_POINT_2F
    {
        point(distance x, distance y)
        : D2D_POINT_2F({ x, y }) {}

        point() : D2D_POINT_2F({ 0, 0 }) {}
    };

    struct line
    {
        point p1;
        point p2;

        line(point const& start, point const& end)
            : p1(start), p2(end) {}

        point mid() const
        {
            return point(
                p1.x + (p2.x - p1.x) / 2,
                p1.y + (p2.y - p1.y) / 2);
        }
    }; 

    struct rectangle : D2D_RECT_F
    {
        rectangle(distance left, distance top, distance right, distance bottom)
            : D2D_RECT_F({ left, top, right, bottom }) {}

        rectangle() : D2D_RECT_F({ 0, 0, 0, 0 }) {}

        distance height() const { return bottom - top; }
        distance width() const { return right - left; }
        point top_left() const { return point(left, top); }
        point bottom_left() const { return point(left, bottom); }
        point top_right() const { return point(right, top); }
        point bottom_right() const { return point(right, bottom); }
        
        line top_edge() const { return line(point(left, top), point(right, top)); }
        line bottom_edge() const { return line(point(left, bottom), point(right, bottom)); }
        line left_edge() const { return line(point(left, top), point(left, bottom)); }
        line right_edge() const { return line(point(right, top), point(right, bottom)); }

        point center() const
        {
            return point(
                left + (right - left) / 2,
                top + (bottom - top) / 2);
        }
    };

    typedef D2D1_COLOR_F color;

    bool empty(rectangle const& r)
    {
        return r.top == r.bottom ||
            r.left == r.right;
    }

    bool contains(rectangle const& r, point const& p)
    {
        return
            r.left <= p.x &&
            r.top <= p.y &&
            r.right > p.x &&
            r.bottom > p.y;
    }

    rectangle from_top(rectangle const& r, distance d)
    {
        return rectangle(
            r.left,
            r.top,
            r.right,
            r.top + d);
    }

    rectangle to_top(rectangle const& r, distance d)
    {
        return rectangle(
            r.left, r.bottom - d,
            r.right, r.bottom);
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

    rectangle above(rectangle const& r, distance d)
    {
        return rectangle(
            r.left,
            r.top,
            r.right,
            std::min(r.bottom, d));
    }

    rectangle above(rectangle const& r, rectangle const& d)
    {
        return rectangle(
            r.left,
            r.top,
            r.right,
            std::min(r.bottom, d.top));
    }

    rectangle from_left(rectangle const& r, distance d)
    {
        return rectangle(
            r.left,
            r.top,
            std::min(r.left + d, r.right),
            r.bottom);
    }

    rectangle to_right(rectangle const& r, distance d)
    {
        return rectangle(
            std::min(r.left + d, r.right),
            r.top,
            r.right,
            r.bottom);
    }

    rectangle to_right(rectangle const& r, rectangle const& d)
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

    point center(rectangle const& r)
    {
        return point(
            r.left + (r.right - r.left) / 2, 
            r.top + (r.bottom - r.top) / 2);
    }

    rectangle centered(rectangle const& r, point const& size)
    {
        point c = center(r);
        return rectangle(
            c.x - size.x / 2,
            c.y - size.y / 2,
            c.x + size.x / 2,
            c.y + size.y / 2);
    }
}