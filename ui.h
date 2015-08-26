#pragma once

#include <boost/thread/once.hpp>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <functional>
#include "drawing.h"
#include "target.h"

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
        std::function<void(drawing::target&)> _onrender;
        drawing::d2d::hwnd_render_target _hwnd_render_target;

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

        void show()
        {
            ShowWindow(_hWnd, SW_SHOWNORMAL);
            UpdateWindow(_hWnd);
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
            wcex.hCursor = NULL;
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
            else
            {
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
    };
}