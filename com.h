#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <exception>

namespace com
{
    template <typename T>
    class com_ptr
    {
        T* ptr;

    public:
        com_ptr() : ptr(nullptr) {}

        com_ptr(T* p) : ptr(p) {}

        ~com_ptr()
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
        T* get() const { return ptr; }
        T* operator->() const { return ptr; }
    };

    class com_exception : public std::exception
    {
    public:
        com_exception(HRESULT hr) {}
    };

    HRESULT throw_call(HRESULT hr)
    {
        if (!SUCCEEDED(hr)) throw com_exception(hr);
        return hr;
    }
}