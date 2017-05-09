#pragma once

#include <Windows.h>

#include <string>
#include <chrono>
#include <functional>
#include <map>

#include <glm/gtc/type_ptr.hpp>

namespace e3d::consts {
  using namespace std::chrono_literals;
  constexpr auto framerate = 60;
  constexpr auto one_second = 1000ms;
  constexpr auto rel_time = one_second / framerate;
}

namespace std {
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
  using message_map_key_t = std::pair<window_t, message_t>;

  using message_map_t = std::multimap<message_map_key_t, message_callback_t>;

  message_map_t& get_message_map();
} // namespace e3d::windows

namespace e3d::windows
{
  HWND create_window(const std::wstring& title);
  bool init_gl(HWND window);
  void message_loop();
}
