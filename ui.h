#pragma once

#include <boost/thread/once.hpp>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <windowsx.h>
#include <functional>
#include "drawing.h"
#include "target.h"
#include <list>

namespace ui
{
    namespace win32
    {
        class win32_exception : public std::exception
        {
        public:
            win32_exception(DWORD error) {}
        };

        template <typename T>
        T throw_null(const T& t)
        {
            if (t == NULL) throw win32_exception(::GetLastError());
            return t;
        }
    }

    static boost::once_flag init_flag;
    
    class window
    {
        HWND _hWnd;
        drawing::d2d::hwnd_render_target _hwnd_render_target;

        typedef std::list<std::function<bool()> > timer_list;
        typedef timer_list::iterator timer_id;

        std::function<void(drawing::target&)> _onrender;
        std::function<void(drawing::point&)> _onpointer;
        std::function<void(drawing::point&)> _onmousedown;
        timer_list _ontimer;

    public:
        window(drawing::factory& f)
        {
            boost::call_once(register_class, init_flag);

            _hWnd = win32::throw_null(::CreateWindowEx(
                WS_EX_OVERLAPPEDWINDOW,
                L"ui::window",
                L"title",
                WS_OVERLAPPEDWINDOW,
                CW_USEDEFAULT, CW_USEDEFAULT,
                300, 400,
                NULL,
                NULL,
                NULL, this));

            _hwnd_render_target.bind_hwnd(f, _hWnd);
        }

        void on_render(std::function<void(drawing::target&)> f)
        {
            _onrender = f;
        }

        void on_pointer(std::function<void(drawing::point&)> f)
        {
            _onpointer = f;
        }

        void on_mousedown(std::function<void(drawing::point&)> f)
        {
            _onmousedown = f;
        }

        timer_id on_timer(std::function<bool()> f)
        {
            if (_ontimer.empty())
                ::SetTimer(_hWnd, 0, 30, NULL);

            _ontimer.push_front(f);
            return _ontimer.begin();
        }

        void off_timer(timer_id id)
        {
            _ontimer.erase(id);

            if (_ontimer.empty())
                ::KillTimer(_hWnd, 0);
        }

        void show()
        {
            ShowWindow(_hWnd, SW_SHOWNORMAL);
            UpdateWindow(_hWnd);
        }

        void redraw()
        {
            ::InvalidateRect(_hWnd, NULL, false);
        }

        void set_timer(UINT timeout)
        {
            ::SetTimer(_hWnd, 0, timeout, NULL);
        }

        void invoke_async(std::function<void()> f)
        {
            auto fp = new std::function<void()>();
            *fp = f;
            ::PostMessage(_hWnd, WM_APP, (UINT_PTR)fp, 0);
        }

    private:
        static void register_class()
        {
            WNDCLASSEX wcex;

            wcex.cbSize = sizeof(WNDCLASSEX);
            wcex.style = 0; // CS_HREDRAW | CS_VREDRAW;
            wcex.lpfnWndProc = WndProc;
            wcex.cbClsExtra = 0;
            wcex.cbWndExtra = 0;
            wcex.hInstance = ::GetModuleHandle(NULL);
            wcex.hIcon = NULL;
            wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
            wcex.hbrBackground = NULL;
            wcex.lpszMenuName = NULL;
            wcex.lpszClassName = L"ui::window";
            wcex.hIconSm = NULL;

            RegisterClassEx(&wcex);
        }

        static window* instance(HWND hWnd)
        {
            return win32::throw_null(reinterpret_cast<window*>(
                ::GetWindowLongPtr(hWnd, GWLP_USERDATA)));
        }

        LRESULT wm_paint(WPARAM wParam, LPARAM lParam)
        {
            if (_onrender)
            {
                _hwnd_render_target.begin_draw();
                _onrender(drawing::target(&_hwnd_render_target));
                ::ValidateRect(_hWnd, NULL);
                _hwnd_render_target.end_draw();
            }
            return 1;
        }

        LRESULT wm_size(WPARAM wParam, LPARAM lParam)
        {
            UINT width = LOWORD(lParam);
            UINT height = HIWORD(lParam);
            _hwnd_render_target.resize(width, height);
            ::InvalidateRect(_hWnd, NULL, false);
            return 1;
        }

        LRESULT wm_mousemove(WPARAM wParam, LPARAM lParam)
        {
            if (_onpointer)
            {
                _onpointer(drawing::point(
                    (drawing::distance)GET_X_LPARAM(lParam),
                    (drawing::distance)GET_Y_LPARAM(lParam)));

                return 0;
            }
            return 0;
        }

        LRESULT wm_lbuttondown(WPARAM wParam, LPARAM lParam)
        {
            if (_onmousedown)
            {
                _onmousedown(drawing::point(
                    (drawing::distance)GET_X_LPARAM(lParam),
                    (drawing::distance)GET_Y_LPARAM(lParam)));

                return 1;
            }
            return 0;
        }

        struct timer_helper
        {
            typedef bool result_type;

            template <typename F>
            bool operator()(F f) const { return !f(); }
        };
        LRESULT wm_timer(WPARAM wParam, LPARAM lParam)
        {
            _ontimer.erase(std::remove_if(_ontimer.begin(), _ontimer.end(), 
                timer_helper()), _ontimer.end());

            return 1;
        }

        LRESULT wm_app(WPARAM wParam, LPARAM lParam)
        {
            auto fp = reinterpret_cast<std::function<void()>*>(wParam);
            (*fp)();
            delete fp;
            return 1;
        }

        static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
        {
            if (message == WM_CREATE)
            {
                LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
                window* w = (window *)pcs->lpCreateParams;

                ::SetWindowLongPtrW(
                    hWnd,
                    GWLP_USERDATA,
                    PtrToUlong(w));
                return 1;
            }
            else if (message == WM_DESTROY)
            {
                PostQuitMessage(0);
                return 1;
            }
            else if (message == WM_PAINT)
            {
                return instance(hWnd)->wm_paint(wParam, lParam);
            }
            else if (message == WM_SIZE)
            {
                return instance(hWnd)->wm_size(wParam, lParam);
            }
            else if (message == WM_MOUSEMOVE)
            {
                return instance(hWnd)->wm_mousemove(wParam, lParam);
            }
            else if (message == WM_LBUTTONDOWN)
            {
                return instance(hWnd)->wm_lbuttondown(wParam, lParam);
            }
            else if (message == WM_TIMER)
            {
                return instance(hWnd)->wm_timer(wParam, lParam);
            }
            else if (message == WM_APP)
            {
                return instance(hWnd)->wm_app(wParam, lParam);
            }
            else
            {
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
    };
}