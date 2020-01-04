
#include "engine.h"

#include "platform.h"

#include "entity.h"
#include "component.h"

// Game engine systems
#include "game_logic_system.h"
#include "rendering_system.h"
#include "collision_system.h"

static bool engine_running = false;

void start_engine()
{
  // Initialization

  init_platform();

  init_entities();
  init_component_pool();

  // Init game engine systems
  init_game_logic_system();



  engine_running = true;

  // Main loop
  while(engine_running)
  {
    // Recieve input
    platform_events();


    float time_step = 16.666f;

    // Process new game state using input
    update_game_logic_system(time_step);
    update_collision_system(time_step);



    // Present the game
    render();
  }

  // Unintialization
  // close_window();
}

void stop_engine()
{
  engine_running = false;
}

