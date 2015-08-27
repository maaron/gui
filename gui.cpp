// gui.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "gui.h"
#include "ui.h"

#include <dwrite.h>
#pragma comment(lib, "dwrite")

#include <vector>
#include <string>
#include <boost/optional.hpp>

using namespace drawing;

drawing::factory f;
text::factory tf;

text::format text_format(tf, L"Arial", NULL,
    DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL,
    DWRITE_FONT_STRETCH_NORMAL, 12.0f, L"en-us");

text::ellipses dots(tf, text_format);

point mouse;
boost::optional<point> click;

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

    bool expanded;

    node(std::wstring const& n) 
        : name(n), expanded(false) {}
};

target draw_tree(target& t, node& tree);
target draw_header(target& t, node& tree);
target draw_node(target& t, node& tree);

target draw_tree(target& t, node& tree)
{
    auto header = draw_header(t, tree);

    auto children = to_right(below(t, header), 10);
    auto remaining = children;

    if (tree.expanded)
    {
        for (auto it = tree.children.begin(); it != tree.children.end(); it++)
        {
            auto child = draw_tree(remaining, *it);
            remaining = below(remaining, child);
            if (empty(remaining)) break;
        }

        draw(t, above(children, remaining), { 0.8, 0.8, 1, 1 });
    }

    return above(t, remaining.top);
}

target draw_expander(target& t, bool expanded)
{
    point p1, p2, p3;
    auto bounds = centered(t, point(8, 8));

    if (!expanded)
    {
        p1 = bounds.top_left();
        p2 = bounds.center();
        p3 = bounds.bottom_left();
    }
    else
    {
        p1 = bounds.top_left();
        p2 = bounds.top_right();
        p3 = bounds.center();
    }

    draw(t, line(p1, p2), { 0, 0, 0, 1 });
    draw(t, line(p2, p3), { 0, 0, 0, 1 });
    draw(t, line(p3, p1), { 0, 0, 0, 1 });

    return t;
}

target draw_header(target& t, node& node)
{
    auto label = draw_node(to_right(t, 15), node);
    auto header = from_left(t, 15);
    header.bottom = label.bottom;
    auto expander = from_left(header, 10);

    if (click && contains(expander, click.get()))
    {
        node.expanded = !node.expanded;
        click = boost::none;
    }

    draw_expander(expander, node.expanded);
    return header;
}

target draw_node(target& t, node& node)
{
    auto used = from_top(t, 20);
    if (contains(used, mouse)) fill(used, { 0.8, 1, 0.8, 1 });
    write_label(t, node.name, centered(used, point(used.width(), 15)));
    return used;
}

target draw_status(target& t)
{
    std::wstring status_text =
        L"Pointer: " +
        std::to_wstring((int)mouse.x) + L", " +
        std::to_wstring((int)mouse.y);

    draw(t, t.top_edge(), { 0, 0, 0, 1 });
    write_label(t, status_text, centered(t, point(t.width(), 12)));
    return t;
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
        fill(t, { 1.0, 1.0, 1.0, 1.0 });

        auto status = to_top(t, 20);
        draw_status(status);

        draw_tree(clip(inside(above(t, status), 5)), root);
    });
    w.on_pointer([&](drawing::point& p)
    {
        mouse = p;
        w.redraw();
    });
    w.on_mousedown([&](drawing::point& p)
    {
        click = p;
        w.redraw();
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
