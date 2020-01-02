
// Interface
#include "game_logic.h"

// Needed modules
#include "game_input.h"
#include "graphics.h"

// Utilities
#include "my_math.h"
#include <stdlib.h> // malloc

struct Player
{
  ModelHandle model;
};

struct Ball
{
  ModelHandle model;
};

struct Wall
{
  ModelHandle model;
};

/*
struct Goal
{
  Model;
};

struct Powerup
{
  Model;
};
*/




struct GameData
{
  Player players[4];

  Ball ball;

  Wall walls[6];
};


static GameData *game_data;


void init_game_logic()
{
  game_data = (GameData *)malloc(sizeof(GameData));

  game_data->players[0].model = create_model(v3(-5.0f,  2.0f, 0.0f), v2(0.7f, 1.0f), 0.0f, Color(0.0f, 0.0f, 1.0f, 1.0f));
  game_data->players[1].model = create_model(v3(-5.0f, -2.0f, 0.0f), v2(0.7f, 1.0f), 0.0f, Color(1.0f, 0.0f, 1.0f, 1.0f));
  game_data->players[2].model = create_model(v3( 5.0f,  2.0f, 0.0f), v2(0.7f, 1.0f), 0.0f, Color(1.0f, 0.0f, 0.0f, 1.0f));
  game_data->players[3].model = create_model(v3( 5.0f, -2.0f, 0.0f), v2(0.7f, 1.0f), 0.0f, Color(1.0f, 1.0f, 0.0f, 1.0f));

  game_data->ball.model = create_model();
}

void update_game_logic()
{
  v2 move_dir = analog_state(0, INPUT_MOVE);
  if(!(move_dir.x == 0.0f && move_dir.y == 0.0f)) move_dir = unit(move_dir);

  float speed = 0.001f;
  change_model_position(game_data->players[0].model, v3(move_dir * speed, 0.0f));

  if(button_state(0, INPUT_SWING))
  {
    set_model_color(game_data->players[0].model, Color(0, 1, 1, 1));
  }
  else
  {
    set_model_color(game_data->players[0].model, Color(0, 0, 1, 1));
  }
}



