
//#include "input.h" // Platform dependent interface
#include "../game_input.h" // Platform independent interface

#include "input.h"
#include "window_handle.h"
#include "renderer.h"

#include <windows.h>
#include <assert.h>


struct InputData
{
  int num_players;
  TickInput current_player_input[4];
  TickInput previous_player_input[4];

  bool current_key_states[256];
  bool previous_key_states[256];
  bool event_key_states[256]; // For recording windows key press events

};
static InputData *input_data;


static void set_button_state(TickInput *input, ButtonInput button)
{
  int byte = button / 8 + 1;
  int bit = 1 << (button % 8);
  input->button_states[byte] |= bit;
}

static bool read_button_state(TickInput *input, ButtonInput button)
{
  int byte = button / 8 + 1;
  int bit = 1 << (button % 8);
  return input->button_states[byte] & bit;
}


// For recording windows key press events
void record_key_event(int vk_code, bool state)
{
  input_data->event_key_states[vk_code] = state;
}

void read_input(unsigned tick_number)
{
  // Only for player 0 for now
  TickInput current_input;
  current_input.tick_number = tick_number;
  current_input.player_number = 0;


  // Read keyboard input
  memcpy(input_data->previous_key_states, input_data->current_key_states, sizeof(bool) * 256);
  memcpy(input_data->current_key_states, input_data->event_key_states, sizeof(bool) * 256);


  // Fill out tick input data
  // Buttons
  if(input_data->current_key_states[' ']) { set_button_state(&current_input, INPUT_SWING); }




  // Analog input
  v2 dir;
  dir.x += (input_data->current_key_states['A']) ? -1.0f : 0.0f;
  dir.x += (input_data->current_key_states['D']) ?  1.0f : 0.0f;
  dir.y += (input_data->current_key_states['S']) ? -1.0f : 0.0f;
  dir.y += (input_data->current_key_states['W']) ?  1.0f : 0.0f;
  current_input.stick_states[0] = dir;

  /*
  v2 player_to_mouse = mouse_world_position() - player_model->position;
  if(length_squared(player_to_mouse) != 0.0f) player->hit_direction = unit(player_to_mouse);
  */


  // Save input
  input_data->previous_player_input[0] = input_data->current_player_input[0];
  input_data->current_player_input[0] = current_input;
}


TickInput get_current_input(int player_number)
{
  assert(player_number < input_data->num_players);
  return input_data->current_player_input[player_number];
}





bool button_toggled_down(int player_id, ButtonInput type)
{
  assert(player_id < input_data->num_players);
  bool down_last_frame = read_button_state(&(input_data->previous_player_input[player_id]), type);
  bool down_this_frame = read_button_state(&(input_data->current_player_input[player_id]), type);

  return (down_last_frame == false && down_this_frame == true) ? true : false;
}

bool button_state(int player_id, ButtonInput type)
{
  assert(player_id < input_data->num_players);
  return read_button_state(&(input_data->previous_player_input[player_id]), type);
}

bool button_toggled_up(int player_id, ButtonInput type)
{
  assert(player_id < input_data->num_players);
  bool down_last_frame = read_button_state(&(input_data->previous_player_input[player_id]), type);
  bool down_this_frame = read_button_state(&(input_data->current_player_input[player_id]), type);

  return (down_last_frame == true && down_this_frame == false) ? true : false;
}



v2 stick_state(int player_id, AnalogInput type)
{
  assert(player_id < input_data->num_players);
  return input_data->current_player_input[player_id].stick_states[type];
}

v2 mouse_world_position()
{
  POINT window_client_pos;
  BOOL success = GetCursorPos(&window_client_pos);
  success = ScreenToClient(get_window_handle(), &window_client_pos);

  return client_to_world(v2((float)window_client_pos.x, (float)window_client_pos.y));
}


bool key_toggled_down(int key)
{
  assert(key < 256);

  return (input_data->previous_key_states[key] == false && input_data->current_key_states[key] == true) ? true : false;
}


void init_input()
{
  input_data = (InputData *)malloc(sizeof(InputData));
  input_data->num_players = 1;
  memset(input_data->current_key_states,  0, sizeof(bool) * 256);
  memset(input_data->previous_key_states, 0, sizeof(bool) * 256);
  memset(input_data->event_key_states,    0, sizeof(bool) * 256);
}

