
//#include "input.h" // Platform dependent interface
#include "../game_input.h" // Platform independent interface

#include "input.h"
#include "window_handle.h"
#include "renderer.h"

#include <windows.h>
#include <assert.h>

struct TickInput
{
  int player_number;

  unsigned char button_states[NUM_BUTTON_INPUTS / 8 + 1]; // 1 bit for each state
  v2 stick_states[NUM_ANALOG_INPUTS];
};

static const int MAX_KEYS = 256;
static const int MAX_MOUSE_KEYS = 8;
struct InputData
{
  int num_players;
  TickInput current_player_input[4];
  TickInput previous_player_input[4];

  bool current_key_states[MAX_KEYS];
  bool previous_key_states[MAX_KEYS];
  bool event_key_states[MAX_KEYS]; // For recording windows key press events

  bool current_mouse_states[MAX_MOUSE_KEYS];
  bool previous_mouse_states[MAX_MOUSE_KEYS];
  bool event_mouse_states[MAX_MOUSE_KEYS]; // For recording windows key press events
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

// For recording windows key press events
void record_mouse_event(int vk_code, bool state)
{
  input_data->event_mouse_states[vk_code] = state;
}

void serialize_input(void **out_stream, int *out_bytes)
{
  *out_stream = malloc(sizeof(TickInput));
  *out_bytes = sizeof(TickInput);

  // Player 0
  memcpy(*out_stream, &(input_data->current_player_input[0]), sizeof(TickInput));
}

void deserialize_input(void *stream, int bytes)
{
  char *stream_pos = (char *)stream;

  int num_input_chunks = bytes / sizeof(TickInput);

  for(int i = 0; i < num_input_chunks; i++)
  {
    TickInput *input = (TickInput *)stream_pos;
    int player_num = input->player_number;
    input_data->current_player_input[player_num] = *input;

    stream_pos += sizeof(TickInput);
  }
}










void read_input()
{
  // Only for player 0 for now
  TickInput current_input;
  current_input.player_number = 0;


  // Read keyboard input
  memcpy(input_data->previous_key_states, input_data->current_key_states, sizeof(bool) * MAX_KEYS);
  memcpy(input_data->current_key_states, input_data->event_key_states, sizeof(bool) * MAX_KEYS);

  // Read mouse input
  memcpy(input_data->previous_mouse_states, input_data->current_mouse_states, sizeof(bool) * MAX_MOUSE_KEYS);
  memcpy(input_data->current_mouse_states, input_data->event_mouse_states, sizeof(bool) * MAX_MOUSE_KEYS);


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

v2 mouse_ndc_position()
{
  POINT window_client_pos;
  BOOL success = GetCursorPos(&window_client_pos);
  success = ScreenToClient(get_window_handle(), &window_client_pos);

  return client_to_ndc(v2((float)window_client_pos.x, (float)window_client_pos.y));
}

bool key_toggled_down(int key)
{
  assert(key < MAX_KEYS);

  return (input_data->previous_key_states[key] == false && input_data->current_key_states[key] == true) ? true : false;
}

bool key_state(int key)
{
  assert(key < MAX_KEYS);

  return input_data->current_key_states[key];
}




bool mouse_toggled_down(int key)
{
  assert(key < MAX_MOUSE_KEYS);

  return (input_data->previous_mouse_states[key] == false && input_data->current_mouse_states[key] == true) ? true : false;
}

bool mouse_state(int key)
{
  assert(key < MAX_MOUSE_KEYS);

  return input_data->current_mouse_states[key];
}


void init_input()
{
  input_data = (InputData *)malloc(sizeof(InputData));
  input_data->num_players = 1;
  memset(input_data->current_key_states,  0, sizeof(bool) * MAX_KEYS);
  memset(input_data->previous_key_states, 0, sizeof(bool) * MAX_KEYS);
  memset(input_data->event_key_states,    0, sizeof(bool) * MAX_KEYS);

  memset(input_data->current_mouse_states,  0, sizeof(bool) * MAX_MOUSE_KEYS);
  memset(input_data->previous_mouse_states, 0, sizeof(bool) * MAX_MOUSE_KEYS);
  memset(input_data->event_mouse_states,    0, sizeof(bool) * MAX_MOUSE_KEYS);
}

