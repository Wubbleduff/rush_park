
#pragma once

#include "my_math.h"

enum ButtonInput
{
  INPUT_SWING,
};

enum AnalogInput
{
  INPUT_MOVE,
  INPUT_AIM,
};


bool button_toggled_down(int player_id, ButtonInput type);
bool button_state(int player_id, ButtonInput type);
bool button_toggled_up(int player_id, ButtonInput type);


v2 analog_state(int player_id, AnalogInput type);


v2 mouse_world_position();

void read_input();


// Debugging
bool key_toggled_down(int button);

