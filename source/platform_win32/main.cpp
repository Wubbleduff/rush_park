
#include "../platform.h"
#include "../engine.h"
#include "window_handle.h"

#include "../game_input.h" // TODO: Move this with other input

#include "renderer.h"
#include "input.h"



#include <windows.h>
#include <assert.h>
#include <stdio.h>

#include <vector> // timers


struct PlatformData
{
  HINSTANCE app_instance;
  HWND window_handle;

  LARGE_INTEGER previous_time;

  std::vector<LARGE_INTEGER> time_stamps;
};

static PlatformData platform_data = {};



// In seconds
void start_profile_timer()
{
  LARGE_INTEGER current_time;
  QueryPerformanceCounter(&current_time);
  platform_data.time_stamps.push_back(current_time);
}
float end_profile_timer()
{
  LARGE_INTEGER counts_per_second;
  QueryPerformanceFrequency(&counts_per_second);

  LARGE_INTEGER current_time;
  QueryPerformanceCounter(&current_time);

  LONGLONG counts_passed = current_time.QuadPart - platform_data.time_stamps.back().QuadPart;
  platform_data.time_stamps.pop_back();

  //double counts_per_ms = (double)counts_per_second.QuadPart / 1000.0;
  double dt = (double)counts_passed / counts_per_second.QuadPart;

  return (float)dt;
}













extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND window, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
  if(ImGui_ImplWin32_WndProcHandler(window, message, wParam, lParam)) return true;

  LRESULT result = 0;

  switch (message)
  {
    case WM_DESTROY:
    {
      PostQuitMessage(0);
      return 0;
    }
    break;

    case WM_CLOSE: 
    {
      DestroyWindow(window);
      return 0;
    }  
    break;
    
    /*
    case WM_PAINT:
    {
      ValidateRect(window_handle, 0);
    }
    break;
    */

    case WM_KEYDOWN: { record_key_event(wParam, true);  break; }
    case WM_KEYUP:   { record_key_event(wParam, false); break; }

    case WM_LBUTTONDOWN: { record_mouse_event(0, true);  break; }
    case WM_LBUTTONUP:   { record_mouse_event(0, false); break; }
    
    default:
    {
      result = DefWindowProc(window, message, wParam, lParam);
    }
    break;
  }
  
  return result;
}

void init_platform()
{
  // Create the window class
  WNDCLASS window_class = {};

  window_class.style = CS_HREDRAW|CS_VREDRAW;
  window_class.lpfnWndProc = WindowProc;
  window_class.hInstance = platform_data.app_instance;
  window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
  window_class.lpszClassName = "Windows Program Class";

  if(!RegisterClass(&window_class)) { return; }

  

#define TRANSPARENT_WINDOWx

  // Create the window
#ifdef TRANSPARENT_WINDOW
  //unsigned monitor_width = GetSystemMetrics(SM_CXSCREEN);
  //unsigned monitor_height = GetSystemMetrics(SM_CYSCREEN);
  unsigned monitor_width = 1024;
  unsigned monitor_height = 768;
  platform_data.window_handle = CreateWindowEx(WS_EX_COMPOSITED,   // Extended style
                                window_class.lpszClassName,        // Class name
                                "First",                           // Window name
                                WS_POPUP | WS_VISIBLE,             // Style of the window
                                0,                                 // Initial X position
                                0,                                 // Initial Y position
                                monitor_width,                     // Initial width
                                monitor_height,                    // Initial height 
                                0,                                 // Handle to the window parent
                                0,                                 // Handle to a menu
                                platform_data.app_instance,        // Handle to an instance
                                0);                                // Pointer to a CLIENTCTREATESTRUCT

  BOOL result = SetWindowLong(platform_data.window_handle, GWL_EXSTYLE, WS_EX_LAYERED) ;
  result = SetLayeredWindowAttributes(platform_data.window_handle, RGB(0, 0, 0), 10, LWA_COLORKEY);
#else
  unsigned width = 1920;
  float ratio = 16.0f / 9.0f;
  unsigned monitor_width = width;
  unsigned monitor_height = (unsigned)((1.0f / ratio) * width);
  platform_data.window_handle = CreateWindowEx(0,                  // Extended style
                                window_class.lpszClassName,        // Class name
                                "First",                           // Window name
                                WS_OVERLAPPEDWINDOW | WS_VISIBLE,  // Style of the window
                                0,                                 // Initial X position
                                0,                                 // Initial Y position
                                monitor_width,                     // Initial width
                                monitor_height,                    // Initial height 
                                0,                                 // Handle to the window parent
                                0,                                 // Handle to a menu
                                platform_data.app_instance,        // Handle to an instance
                                0);                                // Pointer to a CLIENTCTREATESTRUCT
#endif
  if(!platform_data.window_handle) { return; }

  RECT client_rect;
  BOOL success = GetClientRect(platform_data.window_handle, &client_rect);
  init_renderer(platform_data.window_handle, client_rect.right, client_rect.bottom, false, false);

  QueryPerformanceCounter(&platform_data.previous_time);
}

void platform_events()
{
  MSG message;
  while(PeekMessage(&message, 0, 0, 0, PM_REMOVE))
  {
    if(message.message == WM_QUIT)
    {
      stop_engine();
    }

    TranslateMessage(&message);
    DispatchMessage(&message);
  }
}

// Get dt in seconds
float get_dt()
{
  LARGE_INTEGER counts_per_second;
  QueryPerformanceFrequency(&counts_per_second);

  LARGE_INTEGER current_time;
  QueryPerformanceCounter(&current_time);

  LONGLONG counts_passed = current_time.QuadPart - platform_data.previous_time.QuadPart;

  platform_data.previous_time = current_time;

  double dt = (double)counts_passed / counts_per_second.QuadPart;

  return (float)dt;
}


HWND get_window_handle() { return platform_data.window_handle; }

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  platform_data.app_instance = hInstance;

  start_engine();

  return 0;
}

