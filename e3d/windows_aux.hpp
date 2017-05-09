#pragma once

#include <Windows.H>

namespace e3d::windows::detail {

  struct nocopyable
  {
    nocopyable() = default;
    nocopyable(const nocopyable &) = delete;
    nocopyable& operator =(const nocopyable &) = delete;
  };

  class dc_holder : nocopyable
  {
    ::HDC _dc{};
    ::HWND _window;
  public:
    dc_holder(::HWND window)
      :_dc(::GetDC(window))
      , _window(window)
    {
    }
    ~dc_holder()
    {
      ::ReleaseDC(_window, _dc);
    }

    explicit operator HDC() const
    {
      return  _dc;
    }
  };
}
