
#include "engine.h"

#include "platform.h"
#include "game_input.h"

// Game engine systems
#include "game_state_system.h"
#include "rendering_system.h"
#include "player_collision_system.h"
#include "player_controller_system.h"
#include "ball_collision_system.h"

#include "imgui.h" // For displaying dt
#include <stdio.h> // sprintf

static bool engine_running = false;



static float update_time_sum = 0.0f;
static int update_time_counter = 0;

static int update_counter = 0;


static void show_profiling()
{
  ImGui::Begin("Main");

  char text_buffer[128];

  if(update_time_counter >= 10000)
  {
    update_time_sum = 0.0f;
    update_time_counter = 0;
  }

  float average_seconds = update_time_sum / update_time_counter;
  sprintf(text_buffer, "%f ms average", average_seconds * 1000.0f);
  ImGui::Text(text_buffer);

  static int highest = 0;
  if(update_counter > highest) highest = update_counter;
  sprintf(text_buffer, "%d max updates per frame", highest);
  ImGui::Text(text_buffer);

  ImGui::End();

}

void start_engine()
{
  // Initialization

  init_platform();

  init_component_collection();

  // Init game engine systems
  init_game_state_system();



  engine_running = true;

  // Main loop
  while(engine_running)
  {
    // Recieve input
    platform_events();


    float dt = get_dt();
    if(dt > 0.033f) dt = 0.033f;

    static const float TIME_STEP = 0.016f;

    static float engine_counter = 0.0f;
    engine_counter += dt;


    static bool simulating_game = true;


    static const int MAX_UPDATES = 5;
    while(engine_counter >= TIME_STEP && update_counter <= MAX_UPDATES)
    {
      start_profile_timer();
      start_frame();
      show_profiling();



      if(should_restart_game())
      {
        reset_game_state();
      }

      if(key_toggled_down('R'))
      {
        set_game_to_restart();
      }

      if(should_simulate_game()) update_player_collision_system(TIME_STEP);
      if(should_simulate_game()) update_player_controller_system(TIME_STEP);
      if(should_simulate_game()) update_ball_collision_system(TIME_STEP);






      render();

      read_input();


      engine_counter -= TIME_STEP;
      update_counter++;

      float time_passed = end_profile_timer();
      update_time_sum += time_passed;
      update_time_counter++;
    }
    update_counter = 0;

  }

  // Unintialization
  // close_window();
}

void stop_engine()
{
  engine_running = false;
}

