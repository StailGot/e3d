#include <iostream>

#include <Windows.h>
#include <WindowsX.h>

#include <GL/glew.h>
#pragma comment(lib, "opengl32.lib")

#include <tuple>
#include <thread>
#include <map>
#include <array>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "window.hpp"


int main(int argc, char * argv[])
{
  HWND window = HWND(e3d::windows::create_window(L"Amazing window name"));
  HDC dc = ::GetDC(window);
  e3d::windows::init_gl(window);
  ::ShowWindow(window, SW_SHOW);

  auto repain_event = [window] { ::SendMessage(window, WM_PAINT, {}, {}); };

  e3d::windows::get_message_map().insert({ {window, WM_PAINT}, [dc](auto...args) {
    const auto fn_args = std::make_tuple(args...);

    if (std::get<e3d::windows::message_t>(fn_args) == WM_PAINT)
    {
      glm::vec4 color{ 0, 1, .5, 0 };
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

  e3d::windows::message_loop();
  return 0;
}
