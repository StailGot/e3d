#include <iostream>

#include <Windows.h>
#include <WindowsX.h>

#include <GL/glew.h>
#include <GL/wglew.h>
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")

#include <tuple>
#include <thread>
#include <map>
#include <array>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "window.hpp"
#include "gl_aux.hpp"


auto init =
[]
{
  ::glEnable(GL_DEPTH_TEST);
  ::glEnable(GL_PROGRAM_POINT_SIZE);
};

int main(int argc, char * argv[])
{
  int w = 800;
  int h = 600;
  HWND window = HWND(e3d::windows::create_window(L"Amazing window name", w, h));
  HDC dc = ::GetDC(window);
  e3d::windows::init_gl(window);
  ::ShowWindow(window, SW_SHOW);

  std::cout << glGetString(GL_VERSION) << "\n";

  auto repain_event = [window] { ::SendMessage(window, WM_PAINT, {}, {}); };


  std::string shaders_path = R"(C:\dev\emptiness\out\Release\x64\exe\shaders)";
  
  GLuint program = gl::create_program(gl::load_shaders(shaders_path), &std::clog);
  glm::vec2 viewport{ w, h };
  
  auto e = e3d::windows::register_event(window, e3d::windows::message_t(WM_PAINT), [&, dc](auto...args) {
    GLfloat color[] = { 0.85f, 0.85f, 0.85f, 0.f };
    ::glClearBufferfv(GL_COLOR, 0, color);

    GLfloat one[] = { 1.f };
    ::glClearBufferfv(GL_DEPTH, 0, one);

    ::glUseProgram(program);
    
    GLint location = ::glGetUniformLocation(program, "viewport");
    ::glUniform2fv(location, 1, glm::value_ptr(viewport));

    ::glBegin(GL_POINTS);
    ::glVertex2f(0, 0);
    ::glEnd();

    ::SwapBuffers(dc);
  });

  e3d::windows::register_event(window, e3d::windows::message_t(WM_SIZE), [&, dc](auto...args) {
    auto size = std::get<e3d::windows::lparam_t>(std::make_tuple(args...));
    viewport = { LOWORD(size), HIWORD(size) };
    ::glViewport(0, 0, static_cast<GLsizei>(viewport.x), static_cast<GLsizei>(viewport.y));
    std::cout
      << viewport.x << " "
      << viewport.y << std::endl;
  });

  std::thread repaint{ [&repain_event] {
    while (true)
    {
      std::this_thread::sleep_for(e3d::consts::rel_time);
      repain_event();
    }
  } };

  init();

  e3d::windows::message_loop();
  return 0;
}
