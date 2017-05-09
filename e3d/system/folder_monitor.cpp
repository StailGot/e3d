#include "folder_monitor.hpp"

#include <iostream>

#include <thread>
#include <chrono>
#include <filesystem>
#include <windows.h>


namespace sys
{

  folder_monitor::folder_monitor( const std::wstring & folder, callback_t callback )
  {
    set_folder( folder );
    set_callback( std::move( callback ) );
  }


  folder_monitor::~folder_monitor()
  {
    stop();
  }


  void folder_monitor::start()
  {
    if (_alive == false)
      std::thread( [this]
    {
      _alive = true;
      _events_handles [stop_event] = ::CreateEventEx( nullptr, L"", 0, EVENT_ALL_ACCESS );
      _events_handles [change_event] = ::FindFirstChangeNotificationW( _folder.c_str(), true, FILE_NOTIFY_CHANGE_LAST_WRITE );
      do_work();
      _alive = false;
    } ).detach();
  }


  void folder_monitor::start( const std::wstring & folder )
  {
    set_folder( folder );
    start();
  }


  void folder_monitor::set_callback( callback_t callback )
  {
    _callback = std::move( callback );
  }


  void folder_monitor::set_folder( const std::wstring & folder )
  {
    _folder = folder;
  }


  void folder_monitor::stop()
  {
    while (_alive)
      ::SetEvent( _events_handles [stop_event] );

    if (_alive)
      for (auto && handle : _events_handles)
        ::CloseHandle( handle );
  }


  void folder_monitor::do_work()
  {

    bool stop = false;
    using namespace std::chrono;

    auto past = high_resolution_clock::now();

    while (_alive && stop == false)
    {
      auto wait_result = ::WaitForMultipleObjects( (DWORD)std::size( _events_handles ), _events_handles, FALSE, INFINITE );

      switch (wait_result)
      {
        case WAIT_OBJECT_0:
        {
          stop = true;
          std::cout << "stop event" << "\n";
          break;
        }
        case WAIT_OBJECT_0 + 1:
        {
          const auto now = high_resolution_clock::now();
          const auto elapsed = duration_cast<duration<double>>(now - past).count();
          past = now;

          if (elapsed > 0.01)
            notify( elapsed );

          ::FindNextChangeNotification( _events_handles [change_event] );

          break;
        }
        case WAIT_ABANDONED:
        case WAIT_FAILED:
        case WAIT_TIMEOUT:
          stop = true;
          std::cout << "wait fail" << "\n";
          break;
        default:
          break;
      }
    }
  }


  void folder_monitor::notify( double elapsed )
  {
    //std::cout << "event" << "\n";
    //std::cout << elapsed << "\n";

    uint8_t buffer [2048] = {};
    DWORD n_bytes = 0;
    ::ReadDirectoryChangesW( _events_handles [change_event], buffer, (DWORD)std::size( buffer ), FALSE, FILE_NOTIFY_CHANGE_LAST_WRITE, &n_bytes, nullptr, nullptr );
    FILE_NOTIFY_INFORMATION & info = *(FILE_NOTIFY_INFORMATION*)&buffer;

    if (_callback)
      (_callback)(info.FileName);
  }

}