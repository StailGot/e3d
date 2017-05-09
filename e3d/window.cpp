#include "window.hpp"
#include <Windows.H>

#include <GL/glew.h>
#include "windows_aux.hpp"
#include <GL/wglew.h>

namespace e3d::windows::detail {

  HWND create_window(const std::wstring & title, uint32_t w, uint32_t h, WNDPROC window_procedure)
  {
    HWND window{};

    WNDCLASS wc{};
    wc.lpfnWndProc = window_procedure;
    wc.lpszClassName = L"OpenGl 4.5 Window class";
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_NOCLOSE;

    ::RegisterClass(&wc);

    window = ::CreateWindowEx({}, wc.lpszClassName, std::data(title)
      , WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, w, h, {}, {}, {}, {});
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

    if (auto pixelFormat = ::ChoosePixelFormat(dc, &pfd))
      if (SetPixelFormat(dc, pixelFormat, &pfd))
        result = true;

    return result;
  }

  HGLRC init_gl_ext(HDC dc, HGLRC gl_context)
  {
    const int pixel_format_attrib_list[] =
    {
      WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
      WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
      WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
      WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
      WGL_COLOR_BITS_ARB, 32,
      WGL_DEPTH_BITS_ARB, 24,
      WGL_STENCIL_BITS_ARB, 8,
      WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
      WGL_SAMPLES_ARB, 16,
      0
    };

    int context_attribs[] =
    {
      WGL_CONTEXT_MAJOR_VERSION_ARB, consts::major_version,
      WGL_CONTEXT_MINOR_VERSION_ARB, consts::minor_version,
      WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
      0
    };

    HGLRC gl_context_ext{};
    PIXELFORMATDESCRIPTOR pfd{};

    INT pixel_format{};
    UINT num_formats{};
    if (::wglChoosePixelFormatARB(dc, pixel_format_attrib_list, nullptr, 1, &pixel_format, &num_formats))
      if (::DescribePixelFormat(dc, pixel_format, sizeof pfd, &pfd))
        if (::SetPixelFormat(dc, pixel_format, &pfd))
        {
          gl_context_ext = ::wglCreateContextAttribsARB(dc, gl_context, context_attribs);
          ::wglMakeCurrent(dc, gl_context_ext);
        }

    return gl_context_ext;
  }

  HGLRC init_gl_base(HDC dc)
  {
    set_pixel_format(dc);
    HGLRC gl_context = ::wglCreateContext(dc);
    return gl_context;
  }

  bool init_gl(HDC dc)
  {
    bool result = false;

    HWND window = HWND(e3d::windows::create_window(L"null", 1, 1));
    {
      dc_holder tmp_dc{ window };

      HGLRC gl_context = init_gl_base(static_cast<HDC>(tmp_dc));
      result = ::wglMakeCurrent(static_cast<HDC>(tmp_dc), gl_context) && !glewInit();
      HGLRC gl_context_ext = init_gl_ext(dc, gl_context);

      ::wglDeleteContext(gl_context);
    }
    ::DestroyWindow(window);
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
    for (auto && callback : get_message_map().equal_range(message_map_key_t{ window, message_t(message) }))
      if (callback.second)
        callback.second(window, message_t(message), wparam_t(wparam), lparam_t(lparam));
    return ::DefWindowProc(window, message, wparam, lparam);
  };

} // namespace e3d::windows::detail


namespace e3d::windows {

  HANDLE create_window(const std::wstring& title, uint32_t w, uint32_t h)
  {
    return detail::create_window(title, w, h, detail::window_procedure);
  }

  void message_loop()
  {
    return detail::message_loop();
  }

  bool init_gl(HANDLE window)
  {
    detail::dc_holder dc{ ::HWND(window) };
    return detail::init_gl(static_cast<HDC>(dc));
  }

  message_map_t & get_message_map()
  {
    static message_map_t message_map;
    return message_map;
  }

  event_t register_event(window_t window, message_t message, message_callback_t callback)
  {
    return get_message_map().insert({ { window, message }, callback });
  }

  void remove_event(event_t event)
  {
    get_message_map().erase(event);
  }

} // namespace e3d::windows