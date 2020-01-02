
// Interface
#include "game_logic.h"

// Needed modules
#include "entity.h"
#include "game_input.h"
#include "graphics.h"

// Utilities
#include "my_math.h"
#include <stdlib.h> // malloc

struct Player
{
};

struct Ball
{
};

struct Wall
{
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
  EntityHandle players[4];

  EntityHandle ball;

  EntityHandle walls[6];
};


static GameData *game_data;


void init_game_logic()
{
  game_data = (GameData *)malloc(sizeof(GameData));

  game_data->players[0] = create_entity("player 1");
  game_data->players[1] = create_entity("player 2");
  game_data->players[2] = create_entity("player 3");
  game_data->players[3] = create_entity("player 4");
  game_data->ball = create_entity("ball");

  ModelHandle model0 = create_model(v3(-5.0f,  2.0f, 0.0f), v2(0.7f, 1.0f), 0.0f, Color(0.0f, 0.0f, 1.0f, 1.0f));
  ModelHandle model1 = create_model(v3(-5.0f, -2.0f, 0.0f), v2(0.7f, 1.0f), 0.0f, Color(1.0f, 0.0f, 1.0f, 1.0f));
  ModelHandle model2 = create_model(v3( 5.0f,  2.0f, 0.0f), v2(0.7f, 1.0f), 0.0f, Color(1.0f, 0.0f, 0.0f, 1.0f));
  ModelHandle model3 = create_model(v3( 5.0f, -2.0f, 0.0f), v2(0.7f, 1.0f), 0.0f, Color(1.0f, 1.0f, 0.0f, 1.0f));
  add_component(game_data->players[0], CMODEL, model0);
  add_component(game_data->players[1], CMODEL, model1);
  add_component(game_data->players[2], CMODEL, model2);
  add_component(game_data->players[3], CMODEL, model3);

  ModelHandle ball_model = create_model();
  add_component(game_data->ball, CMODEL, ball_model);

}

void update_game_logic()
{
  v2 move_dir = analog_state(0, INPUT_MOVE);
  if(!(move_dir.x == 0.0f && move_dir.y == 0.0f)) move_dir = unit(move_dir);

  ModelHandle player_model = get_component(game_data->players[0], CMODEL);

  float speed = 0.001f;
  change_model_position(player_model, v3(move_dir * speed, 0.0f));

  if(button_state(0, INPUT_SWING))
  {
    set_model_color(player_model, Color(0, 1, 1, 1));
  }
  else
  {
    set_model_color(player_model, Color(0, 0, 1, 1));
  }
}



