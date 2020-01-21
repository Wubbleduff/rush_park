
#include "../platform.h"
#include "../engine.h"

#include "../game_input.h" // TODO: Move this with other input

#include "renderer.h"



#include <windows.h>
#include <assert.h>
#include <stdio.h>


struct PlatformData
{
  HINSTANCE app_instance;
  HWND window_handle;

  bool key_states[256];      // TODO: This is temporary and should be moved to the input module
  bool prev_key_states[256]; // TODO: This is temporary and should be moved to the input module

  LARGE_INTEGER previous_time;

};

static PlatformData platform_data = {};


void read_input()
{
  memcpy(platform_data.prev_key_states, platform_data.key_states, sizeof(platform_data.key_states));
}


bool button_toggled_down(int player_id, ButtonInput type)
{
  int index = 0;

  switch(type)
  {
    case INPUT_SWING: { index = ' '; break; }
  }

  return (platform_data.prev_key_states[index] == false && platform_data.key_states[index] == true) ? true : false;
}

bool button_state(int player_id, ButtonInput type)
{
  int index = 0;

  switch(type)
  {
    case INPUT_SWING: { index = ' '; break; }
  }

  return platform_data.key_states[index];
}

bool button_toggled_up(int player_id, ButtonInput type)
{
  return false;
}



v2 analog_state(int player_id, AnalogInput type)
{
  v2 dir;

  if(type == INPUT_MOVE)
  {
    dir.x += (platform_data.key_states['A']) ? -1.0f : 0.0f;
    dir.x += (platform_data.key_states['D']) ?  1.0f : 0.0f;

    dir.y += (platform_data.key_states['S']) ? -1.0f : 0.0f;
    dir.y += (platform_data.key_states['W']) ?  1.0f : 0.0f;

    return dir;
  }

  return dir;
}



v2 mouse_world_position()
{
  POINT window_client_pos;
  BOOL success = GetCursorPos(&window_client_pos);
  assert(success);
  success = ScreenToClient(platform_data.window_handle, &window_client_pos);
  assert(success);

  return client_to_world(v2(window_client_pos.x, window_client_pos.y));
}







LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
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

    case WM_KEYDOWN: 
    {
      platform_data.key_states[wParam] = true;
    }
    break;

    case WM_KEYUP:
    {
      platform_data.key_states[wParam] = false;
    }
    break;

    /*
    case WM_LBUTTONDOWN:
    {
      mouse_states[0] = true;
    }
    break;

    case WM_LBUTTONUP:
    {
      mouse_states[0] = false;
    }
    break;
    */
    
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
  unsigned monitor_width = GetSystemMetrics(SM_CXSCREEN);
  unsigned monitor_height = GetSystemMetrics(SM_CYSCREEN);
  platform_data.window_handle = CreateWindowEx(0,    // Extended style
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

  if(platform_data.key_states[VK_ESCAPE]) stop_engine();


  QueryPerformanceCounter(&platform_data.previous_time);
}

float get_dt()
{
  LARGE_INTEGER counts_per_second;
  QueryPerformanceFrequency(&counts_per_second);

  LARGE_INTEGER current_time;
  QueryPerformanceCounter(&current_time);

  LONGLONG counts_passed = current_time.QuadPart - platform_data.previous_time.QuadPart;

  double counts_per_ms = (double)counts_per_second.QuadPart / 1000.0;
  double dt = (double)counts_passed / counts_per_ms;

  return (float)dt;
}


int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  platform_data.app_instance = hInstance;

  start_engine();

  return 0;
}

