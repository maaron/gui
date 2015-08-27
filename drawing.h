#pragma once

#include "d2d.h"
#include "target.h"
#include "text.h"
#include <exception>

namespace drawing
{
    void draw(target& t, const rectangle& r, const color& c)
    {
        auto native = t.rtarget->get_target();
        solid_brush b(native, c);
        native->DrawRectangle(r, b);
    }

    void draw(target& t, line const& l, color const& c)
    {
        auto native = t.rtarget->get_target();
        solid_brush b(native, c);
        native->DrawLine(l.p1, l.p2, b);
    }

    void fill(target& t, const rectangle& r, const color& c)
    {
        auto native = t.rtarget->get_target();
        solid_brush b(native, c);
        native->FillRectangle(r, b);
    }

    void fill(target& t, const color& c)
    {
        fill(t, t, c);
    }

    void write(target& t, point const& p, text::layout const& l, const color& c)
    {
        auto native = t.rtarget->get_target();
        solid_brush b(native, c);
        native->DrawTextLayout(p, const_cast<IDWriteTextLayout*>(l.ptr.get()), b, D2D1_DRAW_TEXT_OPTIONS_CLIP);
    }

    struct clip : target
    {
        clip(target const& t) : target(t)
        {
            rtarget->get_target()->PushAxisAlignedClip(
                t, D2D1_ANTIALIAS_MODE_ALIASED);
        }

        ~clip()
        {
            rtarget->get_target()->PopAxisAlignedClip();
        }
    };
}