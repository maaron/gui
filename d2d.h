#pragma once

#include <d2d1.h>
#include <exception>
#include "geometry.h"

#pragma comment(lib, "d2d1")

namespace drawing
{
    namespace d2d
    {
        template <typename T>
        class scoped_resource
        {
            T* ptr;

        public:
            scoped_resource() : ptr(nullptr) {}

            scoped_resource(T* p) : ptr(p) {}

            ~scoped_resource()
            {
                release();
            }

            void release()
            {
                if (ptr != nullptr)
                {
                    ptr->Release();
                    ptr = nullptr;
                }
            }

            T** operator&() { return &ptr; }
            operator T*() { return ptr; }
            T*& get() { return ptr; }
            T* operator->() { return ptr; }
        };

        class d2d_exception : public std::exception
        {
        public:
            d2d_exception(HRESULT hr) {}
        };

        HRESULT throw_call(HRESULT hr)
        {
            if (!SUCCEEDED(hr)) throw d2d_exception(hr);
            return hr;
        }
    }

    struct factory : public d2d::scoped_resource<ID2D1Factory>
    {
        factory()
        {
            d2d::throw_call(D2D1CreateFactory(
                D2D1_FACTORY_TYPE_SINGLE_THREADED,
                &(*this)));
        }
    };

    namespace d2d
    {
        struct render_target
        {
            virtual ID2D1RenderTarget* get_target() = 0;
        };

        class hwnd_render_target : public render_target
        {
            scoped_resource<ID2D1HwndRenderTarget> _resource;
            ID2D1Factory* _factory;
            HWND _hWnd;

            ID2D1RenderTarget* get_target() override { return _resource.get(); }

            void create()
            {
                RECT rc;
                ::GetClientRect(_hWnd, &rc);

                D2D1_SIZE_U size = D2D1::SizeU(
                    rc.right - rc.left,
                    rc.bottom - rc.top);

                throw_call(_factory->CreateHwndRenderTarget(
                    D2D1::RenderTargetProperties(
                        D2D1_RENDER_TARGET_TYPE_DEFAULT, 
                        D2D1::PixelFormat(
                            DXGI_FORMAT_UNKNOWN, 
                            D2D1_ALPHA_MODE_PREMULTIPLIED)),
                    D2D1::HwndRenderTargetProperties(_hWnd, size),
                    &_resource));
            }

            void release()
            {
                _resource.release();
            }

        public:
            void bind_hwnd(ID2D1Factory* factory, HWND hWnd)
            {
                _factory = factory;
                _hWnd = hWnd;

                create();
            }

            void begin_draw()
            {
                if (!_resource)
                {
                    create();
                }
                _resource.get()->BeginDraw();
            }

            void end_draw()
            {
                auto hr = _resource.get()->EndDraw();
                if (hr == D2DERR_RECREATE_TARGET)
                {
                    _resource.release();
                }
            }

            void resize(UINT width, UINT height)
            {
                D2D1_SIZE_U size;
                size.width = width;
                size.height = height;
                auto hr = _resource.get()->Resize(size);
                //InvalidateRect(_hWnd, NULL, false);
            }
        };
    }

    struct solid_brush : d2d::scoped_resource<ID2D1SolidColorBrush>
    {
        solid_brush(ID2D1RenderTarget* t, const color& c)
        {
            d2d::throw_call(t->CreateSolidColorBrush(
                c, &(*this)));
        }
    };
}