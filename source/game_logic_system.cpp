
// Interface
#include "game_logic_system.h"

// Needed components
#include "entity.h"
#include "component.h"

// Utilities
#include "game_input.h"
#include "my_math.h"
#include <stdlib.h> // malloc

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

  ModelComponent *model;
  model = (ModelComponent *)game_data->players[0].get_component(C_MODEL);
  model->position = v2(-5.0f,  2.0f);
  model->scale = player_scale;
  model->color = Color(0.0f, 0.0f, 1.0f, 1.0f);

  model = (ModelComponent *)game_data->players[1].get_component(C_MODEL);
  model->position = v2(-5.0f, -2.0f);
  model->scale = player_scale;
  model->color = Color(0.0f, 1.0f, 1.0f, 1.0f);

  model = (ModelComponent *)game_data->players[2].get_component(C_MODEL);
  model->position = v2( 5.0f,  2.0f);
  model->scale = player_scale;
  model->color = Color(1.0f, 0.0f, 0.0f, 1.0f);

  model = (ModelComponent *)game_data->players[3].get_component(C_MODEL);
  model->position = v2( 5.0f, -2.0f);
  model->scale = player_scale;
  model->color = Color(1.0f, 1.0f, 0.0f, 1.0f);





  game_data->players[0].add_component(C_COLLIDER);
  game_data->players[1].add_component(C_COLLIDER);
  game_data->players[2].add_component(C_COLLIDER);
  game_data->players[3].add_component(C_COLLIDER);

  ColliderComponent *collider;
  collider = (ColliderComponent *)game_data->players[0].get_component(C_COLLIDER);
  collider->rect.scale = player_scale;
  collider->mass = 2.0f;
  collider->is_static = false;

  collider = (ColliderComponent *)game_data->players[1].get_component(C_COLLIDER);
  collider->rect.scale = player_scale;
  collider->mass = 1.0f;
  collider->is_static = false;

  collider = (ColliderComponent *)game_data->players[2].get_component(C_COLLIDER);
  collider->rect.scale = player_scale;
  collider->mass = 1.0f;
  collider->is_static = false;

  collider = (ColliderComponent *)game_data->players[3].get_component(C_COLLIDER);
  collider->rect.scale = player_scale;
  collider->mass = 0.5f;
  collider->is_static = false;



  game_data->players[0].add_component(C_PLAYER);
  game_data->players[1].add_component(C_PLAYER);
  game_data->players[2].add_component(C_PLAYER);
  game_data->players[3].add_component(C_PLAYER);

  PlayerComponent *player;
  player = (PlayerComponent *)game_data->players[0].get_component(C_PLAYER);
  player->num = 0;
  player->character_type = PlayerComponent::CRANKY_S_PANKY;

  player = (PlayerComponent *)game_data->players[1].get_component(C_PLAYER);
  player->num = 1;
  player->character_type = PlayerComponent::RAZZ;

  player = (PlayerComponent *)game_data->players[2].get_component(C_PLAYER);
  player->num = 2;
  player->character_type = PlayerComponent::SCUM;

  player = (PlayerComponent *)game_data->players[3].get_component(C_PLAYER);
  player->num = 3;
  player->character_type = PlayerComponent::TAG;




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

  model = (ModelComponent *)game_data->walls[0].get_component(C_MODEL);
  model->position = v2(-x_pos, -v_y_pos);
  model->scale = vertical_wall_scale;
  model->color = wall_color;

  model = (ModelComponent *)game_data->walls[1].get_component(C_MODEL);
  model->position = v2(-x_pos,  v_y_pos);
  model->scale = vertical_wall_scale;
  model->color = wall_color;

  model = (ModelComponent *)game_data->walls[2].get_component(C_MODEL);
  model->position = v2(0.0f, h_pos);
  model->scale = horizontal_wall_scale;
  model->color = wall_color;

  model = (ModelComponent *)game_data->walls[3].get_component(C_MODEL);
  model->position = v2( x_pos,  v_y_pos);
  model->scale = vertical_wall_scale;
  model->color = wall_color;

  model = (ModelComponent *)game_data->walls[4].get_component(C_MODEL);
  model->position = v2( x_pos, -v_y_pos);
  model->scale = vertical_wall_scale;
  model->color = wall_color;

  model = (ModelComponent *)game_data->walls[5].get_component(C_MODEL);
  model->position = v2(0.0f, -h_pos);
  model->scale = horizontal_wall_scale;
  model->color = wall_color;


  game_data->walls[0].add_component(C_COLLIDER);
  game_data->walls[1].add_component(C_COLLIDER);
  game_data->walls[2].add_component(C_COLLIDER);
  game_data->walls[3].add_component(C_COLLIDER);
  game_data->walls[4].add_component(C_COLLIDER);
  game_data->walls[5].add_component(C_COLLIDER);

  collider = (ColliderComponent *)game_data->walls[0].get_component(C_COLLIDER);
  collider->type = ColliderComponent::RECT;
  collider->rect.scale = vertical_wall_scale;
  collider->mass = 1.0f;
  collider->is_static = true;

  collider = (ColliderComponent *)game_data->walls[1].get_component(C_COLLIDER);
  collider->type = ColliderComponent::RECT;
  collider->rect.scale = vertical_wall_scale;
  collider->mass = 1.0f;
  collider->is_static = true;

  collider = (ColliderComponent *)game_data->walls[2].get_component(C_COLLIDER);
  collider->type = ColliderComponent::RECT;
  collider->rect.scale = horizontal_wall_scale;
  collider->mass = 1.0f;
  collider->is_static = true;

  collider = (ColliderComponent *)game_data->walls[3].get_component(C_COLLIDER);
  collider->type = ColliderComponent::RECT;
  collider->rect.scale = vertical_wall_scale;
  collider->mass = 1.0f;
  collider->is_static = true;

  collider = (ColliderComponent *)game_data->walls[4].get_component(C_COLLIDER);
  collider->type = ColliderComponent::RECT;
  collider->rect.scale = vertical_wall_scale;
  collider->mass = 1.0f;
  collider->is_static = true;

  collider = (ColliderComponent *)game_data->walls[5].get_component(C_COLLIDER);
  collider->type = ColliderComponent::RECT;
  collider->rect.scale = horizontal_wall_scale;
  collider->mass = 1.0f;
  collider->is_static = true;



  game_data->ball.add_component(C_MODEL);
  model = (ModelComponent *)game_data->ball.get_component(C_MODEL);
  model->texture = "ball";
  
  game_data->ball.add_component(C_COLLIDER);
  collider = (ColliderComponent *)game_data->ball.get_component(C_COLLIDER);
  collider->type = ColliderComponent::CIRCLE;
  collider->circle.radius = 0.5f;
  collider->mass = 1.0f;
  collider->is_static = false;

  game_data->ball.add_component(C_BALL);
  BallComponent *ball = (BallComponent *)game_data->ball.get_component(C_BALL);
}

void update_game_logic_system(float time_step)
{
#if 0
  v2 move_dir = analog_state(0, INPUT_MOVE);
  if(!(move_dir.x == 0.0f && move_dir.y == 0.0f)) move_dir = unit(move_dir);

  ModelComponent *player_model = (ModelComponent *game_data->players[0].get_component(C_MODEL);

  float speed = 0.0001f;
  player_model->position += move_dir * speed * time_step;

  if(button_state(0, INPUT_SWING))
  {
    player_model->color = Color(0, 1, 1, 1);
  }
  else
  {
    player_model->color = Color(0, 0, 1, 1);
  }
#endif
}



