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

drawing::factory f;
text::factory tf;

text::format text_format(tf, L"Arial", NULL,
    DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL,
    DWRITE_FONT_STRETCH_NORMAL, 10.0f, L"en-us");

text::ellipses dots(tf, text_format);

auto write_label = [&](target& t, std::wstring const& s, drawing::rectangle const& r)
{
    text::layout l(tf, s, text_format, r.width(), r.height());
    DWRITE_TRIMMING opts;
    opts.delimiter = 0;
    opts.delimiterCount = 0;
    opts.granularity = DWRITE_TRIMMING_GRANULARITY_CHARACTER;
    l.ptr->SetTrimming(&opts, dots.ptr);

    drawing::write(t,
        point(r.left, r.top), l,
        { 0.0, 0.0, 0.0, 1.0 });
};

struct node
{
    std::wstring name;
    std::vector<node> children;

    node(std::wstring const& n) : name(n) {}
};

target draw_tree(target& t, const node& tree);
target draw_header(target& t, const node& tree);
target draw_node(target& t, const node& tree);

target draw_tree(target& t, const node& tree)
{
    auto header = draw_header(t, tree);

    auto children = right_of(below(t, header.bounds), 5);
    auto remaining = children;
    for (auto it = tree.children.begin(); it != tree.children.end(); it++)
    {
        auto child = draw_tree(remaining, *it);
        remaining = below(remaining, child.bounds);
        if (empty(remaining)) break;
    }

    return above(t, remaining.bounds.top);
}

target draw_header(target& t, const node& node)
{
    auto label = draw_node(right_of(t, 10), node);
    auto header = from_top(t, label.bounds.height());
    auto expander = from_left(header, 10);
    draw(t, centered(expander, point(3, 3)).bounds, { 0, 0, 0, 1 });
    return header;
}

target draw_node(target& t, const node& node)
{
    auto used = from_top(t, 14);
    write_label(t, node.name, used.bounds);
    return used;
}

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

    node root(L"root");

    root.children.push_back(node(L"child1"));
    root.children[0].children.push_back(node(L"granchild1 of 1"));
    root.children[0].children.push_back(node(L"granchild2 of 1"));
    root.children.push_back(node(L"child2"));
    root.children[1].children.push_back(node(L"granchild1 of 2"));
    root.children[1].children.push_back(node(L"granchild2 of 2"));
    root.children[1].children.push_back(node(L"granchild3 of 2"));
    root.children.push_back(node(L"child3"));
    root.children[2].children.push_back(node(L"granchild1 of 3"));

    ui::window w(f);
    w.on_render([&](target& t)
    {
        fill(t, t.bounds, { 1.0, 1.0, 1.0, 1.0 });

        draw_tree(t, root);
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
