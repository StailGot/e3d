#pragma once

#include <atomic>
#include <string>
#include <functional>
#include <memory>


namespace sys {

  class folder_monitor
  {
  public:
    using callback_t = std::function<void( const std::wstring & )>;

  private:
    enum events_t
    {
      stop_event, change_event, events_count
    };

    std::wstring _folder;
    void * _events_handles [events_count] = {};

    callback_t _callback;
    std::atomic_bool _alive = false;

  public:
    folder_monitor() = delete;
    folder_monitor( const std::wstring & folder, callback_t callback );
    ~folder_monitor();

    void start();
    void stop();

  private:
    void set_callback( callback_t callback );
    void set_folder( const std::wstring & folder );
    void start( const std::wstring & folder );
    void do_work();
    void notify( double elapsed );
  };

}
