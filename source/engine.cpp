
#include "engine.h"

#include "platform.h"
#include "game_input.h"

// Game engine systems
#include "game_state_system.h"
#include "rendering_system.h"
#include "player_collision_system.h"
#include "player_controller_system.h"
#include "ball_collision_system.h"
#include "networking_system.h"

#include "imgui.h"  // For displaying dt
#include <stdio.h>  // sprintf
#include <stdlib.h> // malloc

enum EngineMode
{
  MENU,

  LOCAL_GAME,
  CLIENT_GAME,
  SERVER_GAME,
};

struct EngineData
{
  bool engine_running = false;
  EngineMode mode;

  float update_time_sum = 0.0f;
  int update_time_counter = 0;

  int update_counter = 0;
};

static EngineData *engine_data;



static void show_profiling()
{
  ImGui::Begin("Main");

  char text_buffer[128];

  if(engine_data->update_time_counter >= 10000)
  {
    engine_data->update_time_sum = 0.0f;
    engine_data->update_time_counter = 0;
  }

  float average_seconds = engine_data->update_time_sum / engine_data->update_time_counter;
  sprintf(text_buffer, "%f ms average", average_seconds * 1000.0f);
  ImGui::Text(text_buffer);

  static int highest = 0;
  if(engine_data->update_counter > highest) highest = engine_data->update_counter;
  sprintf(text_buffer, "%d max updates per frame", highest);
  ImGui::Text(text_buffer);

  ImGui::End();

}

static void simulate_game(float time_step)
{
  if(should_simulate_game()) update_player_collision_system(time_step);
  if(should_simulate_game()) update_player_controller_system(time_step);
  if(should_simulate_game()) update_ball_collision_system(time_step);
}

void start_engine()
{
  // Initialization
  engine_data = (EngineData *)malloc(sizeof(EngineData));
  engine_data->mode = LOCAL_GAME;
  engine_data->update_time_sum = 0.0f;
  engine_data->update_time_counter = 0.0f;
  engine_data->update_counter = 0.0f;



  init_platform();

  // Init game engine systems
  init_component_collection();
  init_game_state_system();
  init_network_system();



  engine_data->engine_running = true;

  // Main loop
  while(engine_data->engine_running)
  {
    // Recieve input
    platform_events();


    // Fixed update data for engine
    float dt = get_dt();
    if(dt > 0.033f) dt = 0.033f;

    static const float TIME_STEP = 0.016f;

    static float engine_counter = 0.0f;
    engine_counter += dt;

    static bool simulating_game = true;


    // Fixed engine tick
    static const int MAX_UPDATES = 5;
    while(engine_counter >= TIME_STEP && engine_data->update_counter <= MAX_UPDATES)
    {

      if(key_toggled_down('1')) engine_data->mode = LOCAL_GAME;
      if(key_toggled_down('2')) engine_data->mode = CLIENT_GAME;
      if(key_toggled_down('3')) engine_data->mode = SERVER_GAME;

      // Update engine systems given the mode
      EngineMode engine_mode = engine_data->mode;
      if(engine_mode == MENU)
      {
        start_frame();
        render();
      }
      else if(engine_mode == LOCAL_GAME)
      {

        start_profile_timer();
        start_frame();
        show_profiling();

        // Check if the user wants to restart the game
        if(should_restart_game())
        {
          reset_game_state();
        }
        if(key_toggled_down('R'))
        {
          set_game_to_restart();
        }

        // Simulate the game systems
        simulate_game(TIME_STEP);

        // Render the game
        render();

        read_input();

        float time_passed = end_profile_timer();
        engine_data->update_time_sum += time_passed;
        engine_data->update_time_counter++;
      }
      else if(engine_mode == CLIENT_GAME)
      {
        start_profile_timer();
        start_frame();
        show_profiling();

        // Send this tick input data
        // send_input_to_server();

        // Recieve new state from the server
        // recieve_game_state();

        // Render the game
        render();

        read_input();

        float time_passed = end_profile_timer();
        engine_data->update_time_sum += time_passed;
        engine_data->update_time_counter++;
      }
      else if(engine_mode == SERVER_GAME)
      {
        // Simulate the game systems
        simulate_game(TIME_STEP);

        // Distribute the game state to clients
        accept_client_connections();
        distribute_game_state();
      }


      engine_counter -= TIME_STEP;
      engine_data->update_counter++;

    }
    engine_data->update_counter = 0;

  }

  // Unintialization
  // close_window();
}

void stop_engine()
{
  engine_data->engine_running = false;
}

