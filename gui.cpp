// gui.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "gui.h"
#include "ui.h"

#include <dwrite.h>
#pragma comment(lib, "dwrite")

#include <vector>
#include <string>

using namespace drawing;

template <typename Tree>
target draw_tree(target& t, const Tree& tree)
{
    auto header = draw_header(t, tree);

    auto children = right_of(below(t, header), 5);
    auto remaining = children;
    for (auto it = tree.begin(); it != tree.end(); it++)
    {
        auto child = draw_tree(remaining, *it);
        remaining = below(remaining, child);
        if (empty(remaining)) break;
    }

    return t;
}

template <typename Node>
target draw_header(target& t, const Node& node)
{
    
}

struct node
{
    std::string name;
    std::vector<node> children;
};

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

    drawing::factory f;
    text::factory tf;

    text::format text_format(tf, L"Arial", NULL, 
        DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, 
        DWRITE_FONT_STRETCH_NORMAL, 10.0f, L"en-us");

    com::com_ptr<IDWriteInlineObject> trim;
    com::throw_call(tf.ptr->CreateEllipsisTrimmingSign(text_format.ptr, &trim));

    auto write_label = [&](target& t, std::wstring const& s, drawing::rectangle const& r)
    {
        text::layout l(tf, s, text_format, r.width(), r.height());
        DWRITE_TRIMMING opts;
        opts.delimiter = 0;
        opts.delimiterCount = 0;
        opts.granularity = DWRITE_TRIMMING_GRANULARITY_CHARACTER;
        l.ptr->SetTrimming(&opts, trim);

        drawing::write(t,
            point(r.left, r.top), l,
            { 0.0, 0.0, 0.0, 1.0 });
    };

    ui::window w(f);
    w.on_render([&](target& t)
    {
        fill(t, t.bounds, { 1.0, 1.0, 1.0, 1.0 });

        rectangle box(20, 10, 130, 30);
        draw(t, box, { 0, 0, 0, 1 });
        write_label(t, L"here's a string that is rather long", inside(box, 5));
    });
    w.show();

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_GUI));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}
