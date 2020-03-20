
#pragma once

#include "my_math.h"

enum ButtonInput
{
  INPUT_SWING,

  NUM_BUTTON_INPUTS
};

enum AnalogInput
{
  INPUT_MOVE,
  INPUT_AIM,

  NUM_ANALOG_INPUTS
};




void read_input();

bool button_toggled_down(int player_id, ButtonInput type);
bool button_state(int player_id, ButtonInput type);
bool button_toggled_up(int player_id, ButtonInput type);


v2 stick_state(int player_id, AnalogInput type);


v2 mouse_world_position();
v2 mouse_ndc_position();



// Debugging
bool key_toggled_down(int button);
bool key_state(int button);

bool mouse_toggled_down(int button);
bool mouse_state(int button);

void init_input();

