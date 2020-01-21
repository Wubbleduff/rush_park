
#include "engine.h"

#include "platform.h"
#include "game_input.h"

#include "entity.h"
#include "component.h"

// Game engine systems
#include "game_logic_system.h"
#include "rendering_system.h"
#include "player_collision_system.h"
#include "player_controller_system.h"
#include "ball_collision_system.h"

static bool engine_running = false;

void start_engine()
{
  // Initialization

  init_platform();

  init_entities();
  init_component_collection();

  // Init game engine systems
  init_game_logic_system();



  engine_running = true;

  // Main loop
  while(engine_running)
  {
    // Recieve input
    platform_events();


    float dt = get_dt();
    if(dt > 0.033f) dt = 0.0033f;

    static const float TIME_STEP = 0.016f;

    static float engine_counter = 0.0f;
    engine_counter += dt;


    while(engine_counter >= TIME_STEP)
    {


      // Process new game state using input
      update_game_logic_system(TIME_STEP);
      update_player_collision_system(TIME_STEP);
      update_player_controller_system(TIME_STEP);
      update_ball_collision_system(TIME_STEP);



      // Present the game
      render();

      read_input();

      engine_counter -= TIME_STEP;
    }

  }

  // Unintialization
  // close_window();
}

void stop_engine()
{
  engine_running = false;
}

