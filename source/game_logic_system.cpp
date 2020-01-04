
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


void init_game_logic_system()
{
  game_data = (TempGameData *)malloc(sizeof(TempGameData));

  game_data->players[0] = create_entity("player 1");
  game_data->players[1] = create_entity("player 2");
  game_data->players[2] = create_entity("player 3");
  game_data->players[3] = create_entity("player 4");
  game_data->ball = create_entity("ball");

  v2 player_scale = v2(0.7f, 1.0f);


  ModelHandle model0 = create_model_component(v2(-5.0f,  2.0f), 0.0f, player_scale, 0.0f, Color(0.0f, 0.0f, 1.0f, 1.0f));
  ModelHandle model1 = create_model_component(v2(-5.0f, -2.0f), 0.0f, player_scale, 0.0f, Color(1.0f, 0.0f, 1.0f, 1.0f));
  ModelHandle model2 = create_model_component(v2( 5.0f,  2.0f), 0.0f, player_scale, 0.0f, Color(1.0f, 0.0f, 0.0f, 1.0f));
  ModelHandle model3 = create_model_component(v2( 5.0f, -2.0f), 0.0f, player_scale, 0.0f, Color(1.0f, 1.0f, 0.0f, 1.0f));

  game_data->players[0].attach_component(model0);
  game_data->players[1].attach_component(model1);
  game_data->players[2].attach_component(model2);
  game_data->players[3].attach_component(model3);


  ModelHandle ball_model = create_model_component();
  game_data->ball.attach_component(ball_model);

  game_data->ball.get_component(C_MODEL);



  ColliderHandle collider0 = create_collider_component();
  collider0->type = ColliderComponent::RECT;
  collider0->rect.scale = player_scale;
  game_data->players[0].attach_component(collider0);

  ColliderHandle collider1 = create_collider_component();
  collider1->type = ColliderComponent::RECT;
  collider1->rect.scale = player_scale;
  game_data->players[1].attach_component(collider1);

}

void update_game_logic_system(float time_step)
{
  v2 move_dir = analog_state(0, INPUT_MOVE);
  if(!(move_dir.x == 0.0f && move_dir.y == 0.0f)) move_dir = unit(move_dir);

#if 1
  ModelHandle player_model = (ModelHandle)game_data->players[0].get_component(C_MODEL);

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



