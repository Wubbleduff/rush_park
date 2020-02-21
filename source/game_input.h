
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

struct TickInput
{
  unsigned tick_number;

  int player_number;

  unsigned char button_states[NUM_BUTTON_INPUTS / 8 + 1]; // 1 bit for each state
  v2 stick_states[NUM_ANALOG_INPUTS];
};



void read_input(unsigned tick_number);

TickInput get_current_input(int player_number);

bool button_toggled_down(int player_id, ButtonInput type);
bool button_state(int player_id, ButtonInput type);
bool button_toggled_up(int player_id, ButtonInput type);


v2 stick_state(int player_id, AnalogInput type);


v2 mouse_world_position();



// Debugging
bool key_toggled_down(int button);

void init_input();

