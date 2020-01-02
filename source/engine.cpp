
#include "engine.h"

#include "platform.h"
#include "entity.h"
#include "game_logic.h"
#include "graphics.h"

static bool engine_running = false;

void start_engine()
{
  // Initialization
  init_platform();
  init_entities();
  init_game_logic();



  engine_running = true;

  // Main loop
  while(engine_running)
  {
    // Recieve input
    platform_events();


    // Process new game state using input
    update_game_logic();



    // Process new engine state using game state



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

