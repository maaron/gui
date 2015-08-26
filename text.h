#pragma once

#include <dwrite.h>
#include <string>
#include "com.h"
#include "geometry.h"

namespace text
{
    struct factory
    {
        com::com_ptr<IDWriteFactory> ptr;

        factory()
        {
            com::throw_call(DWriteCreateFactory(
                DWRITE_FACTORY_TYPE_SHARED,
                __uuidof(IDWriteFactory),
                reinterpret_cast<IUnknown**>(&ptr)));
        }
    };

    struct format
    {
        com::com_ptr<IDWriteTextFormat> ptr;

        format(factory& f,
            WCHAR const* fontName,
            IDWriteFontCollection* fontCollection,
            DWRITE_FONT_WEIGHT fontWeight,
            DWRITE_FONT_STYLE fontStyle,
            DWRITE_FONT_STRETCH fontStretch,
            FLOAT fontSize,
            WCHAR const* localeName)
        {
            com::throw_call(f.ptr->CreateTextFormat(
                fontName, fontCollection, fontWeight, fontStyle, fontStretch, 
                fontSize, localeName, &ptr));
        }
    };

    struct layout
    {
        com::com_ptr<IDWriteTextLayout> ptr;

        layout(factory& f, 
            const std::wstring& string, 
            format& textFormat, 
            drawing::distance maxWidth, 
            drawing::distance maxHeight)
        {
            com::throw_call(f.ptr->CreateTextLayout(
                string.c_str(), string.length(), textFormat.ptr, maxWidth, maxHeight, &ptr));
        }
    };
}