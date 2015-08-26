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

    void fill(target& t, const rectangle& r, const color& c)
    {
        auto native = t.rtarget->get_target();
        solid_brush b(native, c);
        native->FillRectangle(r, b);
    }

    void write(target& t, point const& p, text::layout const& l, const color& c)
    {
        auto native = t.rtarget->get_target();
        solid_brush b(native, c);
        native->DrawTextLayout(p, const_cast<IDWriteTextLayout*>(l.ptr.get()), b, D2D1_DRAW_TEXT_OPTIONS_CLIP);
    }
}