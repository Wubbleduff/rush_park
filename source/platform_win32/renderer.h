#pragma once

#include <windows.h> // HWND



void init_renderer(HWND window, unsigned framebuffer_width, unsigned framebuffer_height, bool is_fullscreen, bool is_vsync);

v2 client_to_world(v2 window_client_pos);
v2 client_to_ndc(v2 window_client_pos);
