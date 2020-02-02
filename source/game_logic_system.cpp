
// Interface
#include "game_logic_system.h"

// Needed components
#include "entity.h"
#include "component.h"

// Utilities
#include "game_input.h"
#include "my_math.h"
#include <stdlib.h> // malloc
#include "imgui.h"

struct TempGameData
{
  EntityID players[4];

  EntityID ball;

  EntityID walls[6];
};


static TempGameData *game_data;



/*
static void collision_callback(CollisionInfo *info)
{
  // ...
}
*/


void init_game_logic_system()
{
  game_data = (TempGameData *)malloc(sizeof(TempGameData));

  game_data->players[0] = create_entity("player 1");
  game_data->players[1] = create_entity("player 2");
  game_data->players[2] = create_entity("player 3");
  game_data->players[3] = create_entity("player 4");
  game_data->walls[0] = create_entity("bottom left wall");
  game_data->walls[1] = create_entity("top left wall");
  game_data->walls[2] = create_entity("top wall");
  game_data->walls[3] = create_entity("top right wall");
  game_data->walls[4] = create_entity("bottom right wall");
  game_data->walls[5] = create_entity("bottom wall");
  game_data->ball = create_entity("ball");
  
  v2 player_scale = v2(0.7f, 1.0f);

  game_data->players[0].add_component(C_MODEL);
  game_data->players[1].add_component(C_MODEL);
  game_data->players[2].add_component(C_MODEL);
  game_data->players[3].add_component(C_MODEL);

  Model *model;
  model = (Model *)game_data->players[0].get_component(C_MODEL);
  model->position = v2(-5.0f,  2.0f);
  model->scale = player_scale;
  model->color = Color(0.0f, 0.0f, 1.0f, 1.0f);

  model = (Model *)game_data->players[1].get_component(C_MODEL);
  model->position = v2(-5.0f, -2.0f);
  model->scale = player_scale;
  model->color = Color(0.0f, 1.0f, 1.0f, 1.0f);

  model = (Model *)game_data->players[2].get_component(C_MODEL);
  model->position = v2( 5.0f,  2.0f);
  model->scale = player_scale;
  model->color = Color(1.0f, 0.0f, 0.0f, 1.0f);

  model = (Model *)game_data->players[3].get_component(C_MODEL);
  model->position = v2( 5.0f, -2.0f);
  model->scale = player_scale;
  model->color = Color(1.0f, 1.0f, 0.0f, 1.0f);






  game_data->players[0].add_component(C_PLAYER);
  game_data->players[1].add_component(C_PLAYER);
  game_data->players[2].add_component(C_PLAYER);
  game_data->players[3].add_component(C_PLAYER);

  Player *player;
  player = (Player *)game_data->players[0].get_component(C_PLAYER);
  player->num = 0;
  player->character_type = Player::CRANKY_S_PANKY;

  player = (Player *)game_data->players[1].get_component(C_PLAYER);
  player->num = 1;
  player->character_type = Player::RAZZ;

  player = (Player *)game_data->players[2].get_component(C_PLAYER);
  player->num = 2;
  player->character_type = Player::SCUM;

  player = (Player *)game_data->players[3].get_component(C_PLAYER);
  player->num = 3;
  player->character_type = Player::TAG;




  float x_pos = 9.5f;
  float v_y_pos = 3.0f;
  float h_pos = 5.0f;

  v2 horizontal_wall_scale = v2(20.0f, 1.0f);
  v2 vertical_wall_scale = v2(1.0f, 4.0f);

  Color wall_color = Color(0.4f, 0.2f, 0.2f, 1.0f);

  game_data->walls[0].add_component(C_MODEL);
  game_data->walls[1].add_component(C_MODEL);
  game_data->walls[2].add_component(C_MODEL);
  game_data->walls[3].add_component(C_MODEL);
  game_data->walls[4].add_component(C_MODEL);
  game_data->walls[5].add_component(C_MODEL);

  model = (Model *)game_data->walls[0].get_component(C_MODEL);
  model->position = v2(-x_pos, -v_y_pos);
  model->scale = vertical_wall_scale;
  model->color = wall_color;

  model = (Model *)game_data->walls[1].get_component(C_MODEL);
  model->position = v2(-x_pos,  v_y_pos);
  model->scale = vertical_wall_scale;
  model->color = wall_color;

  model = (Model *)game_data->walls[2].get_component(C_MODEL);
  model->position = v2(0.0f, h_pos);
  model->scale = horizontal_wall_scale;
  model->color = wall_color;

  model = (Model *)game_data->walls[3].get_component(C_MODEL);
  model->position = v2( x_pos,  v_y_pos);
  model->scale = vertical_wall_scale;
  model->color = wall_color;

  model = (Model *)game_data->walls[4].get_component(C_MODEL);
  model->position = v2( x_pos, -v_y_pos);
  model->scale = vertical_wall_scale;
  model->color = wall_color;

  model = (Model *)game_data->walls[5].get_component(C_MODEL);
  model->position = v2(0.0f, -h_pos);
  model->scale = horizontal_wall_scale;
  model->color = wall_color;

  game_data->walls[0].add_component(C_WALL);
  game_data->walls[1].add_component(C_WALL);
  game_data->walls[2].add_component(C_WALL);
  game_data->walls[3].add_component(C_WALL);
  game_data->walls[4].add_component(C_WALL);
  game_data->walls[5].add_component(C_WALL);


  game_data->ball.add_component(C_MODEL);
  model = (Model *)game_data->ball.get_component(C_MODEL);
  model->texture = "ball";

  game_data->ball.add_component(C_BALL);
  Ball *ball = (Ball *)game_data->ball.get_component(C_BALL);
}

void update_game_logic_system(float time_step)
{
}



