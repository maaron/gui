// gui.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "gui.h"
#include "ui.h"
#include "tree.h"

#include <dwrite.h>
#pragma comment(lib, "dwrite")

#include <iostream>
#include <vector>
#include <list>
#include <deque>
#include <string>
#include <thread>
#include <boost/optional.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>

using namespace drawing;

boost::asio::io_service io;
boost::asio::deadline_timer timer(io);
drawing::factory f;
text::factory tf;
ui::window w(f);

text::format text_format(tf, L"Arial", NULL,
    DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL,
    DWRITE_FONT_STRETCH_NORMAL, 12.0f, L"en-us");

text::ellipses dots(tf, text_format);

point mouse;
boost::optional<point> click;
std::list<std::function<void()> > timers;

void animate(boost::asio::deadline_timer& timer, std::function<bool()> f)
{
    std::function<void(const boost::system::error_code&)> timer_func;
    
    auto ui_func = [f, &timer]()
    {
        if (f())
        {
            animate(timer, f);
        }
    };
    timer_func = [f, ui_func](const boost::system::error_code& error)
    {
        if (error) return;
        w.invoke_async(ui_func);
    };
    timer.expires_from_now(boost::posix_time::milliseconds(50));
    timer.async_wait(timer_func);
}

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
    typedef std::list<node>::iterator iterator;

    std::wstring name;
    std::list<node> children;

    enum { collapsed, expanding, expanded, collapsing } state;
    std::chrono::monotonic_clock::time_point start;
    degrees expander_angle;

    bool is_expanded()
    {
        return state == expanded ||
            state == collapsing;
    }

    void click()
    {
        switch (state)
        {
        case collapsed: 
            state = expanding;
            start = std::chrono::monotonic_clock::now();
            w.on_timer([this](){ w.redraw(); return state == expanding; });
            break;

        case expanded: 
            state = collapsing;
            start = std::chrono::monotonic_clock::now();
            w.on_timer([this](){ w.redraw(); return state == collapsing; });
            break;
        }
    }

    void update()
    {
        const std::chrono::duration<float> d(0.1);
        auto now = std::chrono::monotonic_clock::now();
        std::chrono::duration<float> elapsed = now - start;
        auto ratio = elapsed.count() / d.count();

        switch (state)
        {
        case expanding:
            if (elapsed > d)
            {
                state = expanded;
                expander_angle = 90;
            }
            else expander_angle = 90 * ratio;
            break;

        case collapsing:
            if (elapsed > d)
            {
                state = collapsed;
                expander_angle = 0;
            }
            else expander_angle = 90 - 90 * ratio;
        }
    }

    node(std::wstring const& n) 
        : name(n), state(collapsed), expander_angle(0) {}
};

// Placeholding class for tree structures that have persistent iterators.
struct tree_view
{
    typedef std::deque<node::iterator>::iterator path_iterator;
    std::deque<node::iterator> path;
    node root;

    tree_view() : root(L"") {}
};

target draw_tree(target& t, node& tree);
target draw_header(target& t, node& tree);
target draw_node(target& t, node& tree);

target draw_children(target& t, node::iterator it, node::iterator end)
{
    auto remaining = t;
    for (; it != end; it++)
    {
        auto child = draw_tree(remaining, *it);
        remaining = below(remaining, child);
        if (empty(remaining)) break;
    }
    return above(t, remaining);
}

target draw_tree(target& t, node& tree)
{
    auto header = draw_header(t, tree);

    auto children = to_right(below(t, header), 10);

    if (tree.is_expanded())
    {
        children = draw_children(children, 
            tree.children.begin(), tree.children.end());

        draw(t, children, { 0.8, 0.8, 1, 1 });

        return children;
    }
    else return header;
}

target draw_expander(target& t, node& node)
{
    point p1, p2, p3;
    auto bounds = centered(t, point(8, 8));

    p1 = bounds.top_left();
    p2 = bounds.center();
    p3 = bounds.bottom_left();

    transform rot(t, D2D1::Matrix3x2F::Rotation(node.expander_angle, center(bounds)));
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
        node.click();
        click = boost::none;
    }
    node.update();

    draw_expander(expander, node);
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

target draw_tree_view_depth(target& t, tree_view& view, tree_view::path_iterator path)
{
    auto remaining = t;
    if (path != view.path.end())
    {
        auto used = draw_tree_view_depth(to_right(t, 10), view, path + 1);
        remaining = below(remaining, used);
    }

    draw_children(below(t, remaining), *path, (*(path - 1))->children.end());

    return t;
}

target draw_tree_view(target& t, tree_view& view)
{
    return draw_tree_view_depth(t, view, view.path.begin() + 1);
}

struct mynode
{
    std::wstring name;
};

template <typename Container>
struct tree
{
    typedef Container container;
    typedef typename Container::value_type value_type;

    value_type value;
    container children;
};

template <typename T>
struct vector_tree : public tree<std::vector<T> > {};

typedef vector_tree<mynode> node_tree;

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

    node_tree nt;
    nt.value.name = L"root";

    ui::depth_first_tree_view<node_tree> tview(nt);
    for (auto& n : tview)
    {
        std::wcout << n.name << std::endl;
    }

    boost::asio::io_service::work work(io);
    std::thread io_thread([&](){ io.run(); });

    node root(L"root");

    root.children.push_back(node(L"child1"));
    auto child = root.children.rbegin();
    child->children.push_back(node(L"granchild1 of 1"));
    child->children.push_back(node(L"granchild2 of 1"));
    
    root.children.push_back(node(L"child2"));
    child = root.children.rbegin();
    child->children.push_back(node(L"granchild1 of 2"));
    child->children.push_back(node(L"granchild2 of 2"));
    child->children.push_back(node(L"granchild3 of 2"));

    root.children.push_back(node(L"child3"));
    child = root.children.rbegin();
    child->children.push_back(node(L"granchild1 of 3"));

    tree_view tv;
    tv.root.children.push_back(root);
    tv.path.push_back(tv.root.children.begin());

    w.on_render([&](target& t)
    {
        fill(t, { 1.0, 1.0, 1.0, 1.0 });

        auto status = to_top(t, 20);
        draw_status(status);

        //draw_tree(clip(inside(above(t, status), 5)), root);
        draw_tree_view(clip(inside(above(t, status), 5)), tv);
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

    io.stop();
    io_thread.join();

	return (int) msg.wParam;
}
