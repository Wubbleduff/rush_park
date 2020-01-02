
#pragma once

#include "my_math.h"

typedef int ModelHandle;

void render();
void shutdown_renderer();

ModelHandle create_model(v3 position = v3(), v2 scale = v2(1.0f, 1.0f), float rotation = 0.0f,
                         Color color = Color(1.0f, 1.0f, 1.0f, 1.0f));

void set_model_position(ModelHandle model, v3 pos);
v3   get_model_position(ModelHandle model);
void change_model_position(ModelHandle model, v3 offset);

void set_model_scale(ModelHandle model, v2 scale);
v2   get_model_scale(ModelHandle model);
void change_model_scale(ModelHandle model, v2 scale);

void set_model_rotation(ModelHandle model, float rotation);
float   get_model_rotation(ModelHandle model);
void change_model_rotation(ModelHandle model, float rotation);

void set_model_color(ModelHandle model, Color color);



void set_camera_position(v2 position);
v2 get_camera_position();


