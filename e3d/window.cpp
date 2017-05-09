#include "window.hpp"
#include <Windows.H>

#include <GL/glew.h>
#include "windows_aux.hpp"


namespace e3d::windows::detail {

  HWND create_window(const std::wstring & title, WNDPROC window_procedure)
  {
    HWND window{};

    WNDCLASS wc{};
    wc.lpfnWndProc = window_procedure;
    wc.lpszClassName = L"OpenGl 4.5 Window class";
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_NOCLOSE;

    ::RegisterClass(&wc);

    window = ::CreateWindowEx({}, wc.lpszClassName, std::data(title)
      ,  WS_OVERLAPPEDWINDOW , CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, {}, {}, {}, {});
    return window;
  }

  bool set_pixel_format(HDC dc)
  {
    bool result = false;
    PIXELFORMATDESCRIPTOR pfd{ sizeof pfd };
    pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.nVersion = 1;

    if (auto pixelFormat = ChoosePixelFormat(dc, &pfd))
      if (SetPixelFormat(dc, pixelFormat, &pfd))
        result = true;

    return result;
  }

  bool init_gl(HDC dc)
  {
    bool result = false;
    set_pixel_format(dc);
    HGLRC gl_context = wglCreateContext(dc);
    result = wglMakeCurrent(dc, gl_context) && !glewInit();
    return result;
  }

  bool init_vulcan(HDC dc)
  {
    throw std::exception("Not implemented");
  }

  void message_loop()
  {
    MSG msg{};
    while (::GetMessage(&msg, {}, {}, {}))
      TranslateMessage(&msg), ::DispatchMessage(&msg);
  }

  LRESULT WINAPI window_procedure(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
  {
    for (auto && callback : get_message_map().equal_range( message_map_key_t{ window,message }))
      if (callback.second)
        callback.second(window, message, wparam, lparam);
    return ::DefWindowProc(window, message, wparam, lparam);
  };

} // namespace e3d::windows::detail


namespace e3d::windows {

  HWND windows::create_window(const std::wstring& title)
  {
    return detail::create_window(title, detail::window_procedure);
  }

  void windows::message_loop()
  {
    return detail::message_loop();
  }

  bool windows::init_gl(HWND window)
  {
    detail::dc_holder dc{ window };
    return detail::init_gl(static_cast<HDC>(dc));
  }

  message_map_t& get_message_map()
  {
    static std::multimap<message_map_key_t, message_callback_t> message_map;
    return message_map;
  }

} // namespace e3d::windows