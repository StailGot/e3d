#include <iostream>

#include <Windows.h>
#include <WindowsX.h>

#include <GL/glew.h>
#include <GL/wglew.h>

#include <tuple>
#include <thread>
#include <unordered_map>
#include <map>
#include <array>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#pragma comment(lib, "opengl32.lib")

namespace std
{
  template <typename T1, typename T2>
  auto begin(const std::pair<T1, T2> & range)
  {
    return range.first;
  }

  template <typename T1, typename T2>
  auto end(const std::pair<T1, T2> & range)
  {
    return range.second;
  }

  template<typename T>
  auto data(T && value) -> decltype(glm::value_ptr(value))
  {
    return glm::value_ptr(value);
  }
}


namespace e3d::windows {
  using message_t = UINT;
  using window_t = HWND;
  using message_callback_t = std::function<void(HWND, UINT, WPARAM, LPARAM)>;
  using key_value_t = std::pair<window_t, message_t>;

  using message_map_t = std::multimap<key_value_t, message_callback_t>;

  message_map_t & get_message_map()
  {
    static std::multimap<key_value_t, message_callback_t> message_map;
    return message_map;
  }

} // namespace e3d::windows


namespace e3d::windows::detail {

  HWND _create_window(const std::wstring & title, WNDPROC window_procedure)
  {
    HWND window{};

    WNDCLASS wc{};
    wc.lpfnWndProc = window_procedure;
    wc.lpszClassName = L"OpenGl 4.5 Window class";
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_NOCLOSE;

    ::RegisterClass(&wc);

    window = ::CreateWindowEx({}, wc.lpszClassName, std::data(title)
      , {}/*WS_VISIBLE*/, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, {}, {}, {}, {});
    return window;
  }

  bool _set_pixel_format(HDC dc)
  {
    bool result = false;
    PIXELFORMATDESCRIPTOR pfd{ sizeof pfd };
    pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.nVersion = 1;

    if (auto pixelFormat = ::ChoosePixelFormat(dc, &pfd))
      if (::SetPixelFormat(dc, pixelFormat, &pfd))
        result = true;

    return result;
  }

  bool _init_gl(HDC dc)
  {
    bool result = false;
    _set_pixel_format(dc);
    HGLRC gl_context = ::wglCreateContext(dc);
    result = ::wglMakeCurrent(dc, gl_context) && !::glewInit();
    return result;
  }

  void _message_loop()
  {
    MSG msg{};
    while (::GetMessage(&msg, {}, {}, {}))
      ::TranslateMessage(&msg), ::DispatchMessage(&msg);
  }

  LRESULT WINAPI _window_procedure(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
  {
    for (auto && callback : get_message_map().equal_range({ window,message }))
      if (callback.second)
        callback.second(window, message, wparam, lparam);
    return ::DefWindowProc(window, message, wparam, lparam);
  };
} // namespace e3d::windows::detail


namespace e3d::windows
{
  HWND create_window(const std::wstring & title)
  {
    return e3d::windows::detail::_create_window(title, e3d::windows::detail::_window_procedure);
  }

  bool init_gl(HWND window)
  {
    bool result = false;
    HDC dc = ::GetDC(window);
    result = detail::_init_gl(dc);
    ::ReleaseDC(window, dc);
    return result;
  }

  void message_loop()
  {
    return detail::_message_loop();
  }
}


namespace e3d::consts
{
  using namespace std::chrono_literals;
  constexpr auto framerate = 60;
  constexpr auto one_second = 1000ms;
  constexpr auto rel_time = one_second / framerate;
}


int main(int argc, char * argv[])
{
  HWND window = e3d::windows::create_window(L"Amazing window name");
  HDC dc = ::GetDC(window);
  e3d::windows::init_gl(window);
  ::ShowWindow(window, SW_SHOW);

  auto repain_event = [window] { ::SendMessage(window, WM_PAINT, {}, {}); };

  e3d::windows::get_message_map().insert({ {window, WM_PAINT}, [dc](auto...args) {
    const auto fn_args = std::make_tuple(args...);

    if (std::get<UINT>(fn_args) == WM_PAINT)
    {
      glm::vec4 color{ 0, 1, 0, 0 };
      ::glClearBufferfv(GL_COLOR, 0, std::data(color) );
      ::SwapBuffers(dc);
    }
  } });

  std::thread repaint{ [&repain_event] {
    while (true)
    {
      std::this_thread::sleep_for(e3d::consts::rel_time);
      repain_event();
    }
  } };

  GLuint buffer{};
  ::glCreateBuffers(1, &buffer);

  e3d::windows::message_loop();
  return 0;
}
