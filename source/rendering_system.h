
#pragma once

#include "component.h"

#include "my_math.h"


enum DebugDrawMode
{
  FILLED,
  WIREFRAME,
};


void debug_draw_rect(v2 position, v2 scale, float rotation, Color color = Color(0.0f, 1.0f, 0.0f), DebugDrawMode draw_mode = WIREFRAME);
void debug_draw_circle(v2 position, float radius, Color color = Color(0.0f, 1.0f, 0.0f), DebugDrawMode draw_mode = WIREFRAME);

void  set_camera_position(v2 position);
v2    get_camera_position();

void start_frame();
void render();

void shutdown_renderer();

