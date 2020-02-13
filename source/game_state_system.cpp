
// Interface
#include "game_state_system.h"
#include "entity.h"
#include "component.h"

// Needed components
#include "entity.h"
#include "component.h"

// Utilities
#include "game_input.h"
#include "my_math.h"
#include "imgui.h"
#include <stdlib.h> // malloc
#include <assert.h>

struct Entity
{
  const char *name;

  Component *components[NUM_COMPONENTS];

  bool destroyed = false;
};

static const int MAX_ENTITIES = 1024;
struct EntityPool
{
  int num_entities;
  Entity *entities;
};

struct GameState
{
  EntityPool *entity_pool;

  bool restarting_game;
  bool simulate_game;
};

static GameState *game_state;







Component *EntityID::add_component(ComponentType type)
{
  Component *component = create_component(EntityID(id), type);
  game_state->entity_pool->entities[id].components[type] = component;
  return component;
}

void EntityID::remove_component(ComponentType type)
{
  Component *component_to_remove = game_state->entity_pool->entities[id].components[type];
  destroy_component(component_to_remove);
  game_state->entity_pool->entities[id].components[type] = nullptr;
}

Component *EntityID::get_component(ComponentType component_type) const
{
  return game_state->entity_pool->entities[id].components[component_type];
}

Model *EntityID::get_model() const
{
  return (Model *)game_state->entity_pool->entities[id].components[C_MODEL];
}

Ball *EntityID::get_ball() const
{
  return (Ball *)game_state->entity_pool->entities[id].components[C_BALL];
}

Wall *EntityID::get_wall() const
{
  return (Wall *)game_state->entity_pool->entities[id].components[C_WALL];
}

Player *EntityID::get_player() const
{
  return (Player *)game_state->entity_pool->entities[id].components[C_PLAYER];
}

bool EntityID::operator==(const EntityID &other) const { return id == other.id; }

EntityID create_entity(const char *name)
{
  if(game_state->entity_pool->num_entities >= MAX_ENTITIES) assert(0);

  Entity entity;

  entity.name = name;
  for(int i = 0; i < NUM_COMPONENTS; i++) entity.components[i] = nullptr;

  game_state->entity_pool->entities[game_state->entity_pool->num_entities] = entity;
  game_state->entity_pool->num_entities++;
  return EntityID(game_state->entity_pool->num_entities - 1);
}

void destroy(EntityID entity)
{
  game_state->entity_pool->entities[entity.id].destroyed = true;
}









void destroy_all_entities()
{
  //for(int i = 0; i < game_state->game_state->entity_pool->num_entities; i++) destroy(game_state->game_state->entity_pool->entities);
}

void create_initial_entities()
{
  EntityID players[4];
  players[0] = create_entity("player 1");
  players[1] = create_entity("player 2");
  players[2] = create_entity("player 3");
  players[3] = create_entity("player 4");

  EntityID walls[4];
  walls[0] = create_entity("bottom left wall");
  walls[1] = create_entity("top left wall");
  walls[2] = create_entity("top wall");
  walls[3] = create_entity("top right wall");
  walls[4] = create_entity("bottom right wall");
  walls[5] = create_entity("bottom wall");
  walls[6] = create_entity("middle left wall");
  walls[7] = create_entity("middle right wall");

  EntityID goals[2];
  goals[0] = create_entity("left goal");
  goals[1] = create_entity("right goal");

  EntityID ball = create_entity("ball");
  


  for(int i = 0; i < sizeof(players) / sizeof(players[0]); i++)
  {
    players[i].add_component(C_MODEL);
    players[i].add_component(C_PLAYER);

    Model *model = (Model *)players[i].get_component(C_MODEL);
    Player *player = (Player *)players[i].get_component(C_PLAYER);

    v2 player_scale = v2(0.7f, 1.0f);
    if(i == 0)
    {
      model->position = v2(-5.0f,  2.0f);
      model->depth = -0.1f;
      model->scale = player_scale;
      model->color = Color(0.0f, 0.0f, 1.0f, 1.0f);

      player->num = 0;
      player->character_type = Player::CRANKY_S_PANKY;
    }
    else if(i == 1)
    {
      model->position = v2(-5.0f, -2.0f);
      model->depth = -0.1f;
      model->scale = player_scale;
      model->color = Color(0.0f, 1.0f, 1.0f, 1.0f);

      player->num = 1;
      player->character_type = Player::RAZZ;
    }
    else if(i == 2)
    {
      model->position = v2( 5.0f,  2.0f);
      model->depth = -0.1f;
      model->scale = player_scale;
      model->color = Color(1.0f, 0.0f, 0.0f, 1.0f);

      player->num = 2;
      player->character_type = Player::SCUM;
    }
    else if(i == 3)
    {
      model->position = v2( 5.0f, -2.0f);
      model->depth = -0.1f;
      model->scale = player_scale;
      model->color = Color(1.0f, 1.0f, 0.0f, 1.0f);

      player->num = 3;
      player->character_type = Player::TAG;
    }
  }



  for(int i = 0; i < sizeof(walls) / sizeof(walls[0]); i++)
  {
    walls[i].add_component(C_MODEL);
    walls[i].add_component(C_WALL);

    Model *model = (Model *)walls[i].get_component(C_MODEL);

    float x_pos = 9.5f;
    float v_y_pos = 3.0f;
    float h_pos = 5.0f;
    v2 horizontal_wall_scale = v2(20.0f, 1.0f);
    v2 vertical_wall_scale = v2(1.0f, 3.0f);
    Color wall_color = Color(0.4f, 0.2f, 0.2f, 1.0f);

    if(i == 0)
    {
      model->position = v2(-x_pos, -v_y_pos);
      model->scale = vertical_wall_scale;
      model->color = wall_color;
    }
    else if(i == 1)
    {
      model->position = v2(-x_pos,  v_y_pos);
      model->scale = vertical_wall_scale;
      model->color = wall_color;
    }
    else if(i == 2)
    {
      model->position = v2(0.0f, h_pos);
      model->scale = horizontal_wall_scale;
      model->color = wall_color;
    }
    else if(i == 3)
    {
      model->position = v2( x_pos,  v_y_pos);
      model->scale = vertical_wall_scale;
      model->color = wall_color;
    }
    else if(i == 4)
    {
      model->position = v2( x_pos, -v_y_pos);
      model->scale = vertical_wall_scale;
      model->color = wall_color;
    }
    else if(i == 5)
    {
      model->position = v2(0.0f, -h_pos);
      model->scale = horizontal_wall_scale;
      model->color = wall_color;
    }
  }



  for(int i = 0; i < sizeof(goals) / sizeof(goals[0]); i++)
  {
    goals[i].add_component(C_MODEL);
    goals[i].add_component(C_GOAL);

    Model *model = (Model *)goals[i].get_component(C_MODEL);

    float x_pos = 9.5f;

    if(i == 0)
    {
      model->position = v2(-x_pos - 0.5f, 0.0f);
      model->scale = v2(1.0f, 3.0f);
      model->color = Color(0.5f, 1.0f, 0.0f);
    }
    else if(i == 1)
    {
      model->position = v2(x_pos + 0.5f, 0.0f);
      model->scale = v2(1.0f, 3.0f);
      model->color = Color(1.0f, 0.0f, 0.5f);
    }
  }



  {
    ball.add_component(C_MODEL);
    ball.add_component(C_BALL);
    Model *model = (Model *)ball.get_component(C_MODEL);

    model->texture = "ball";

    //Ball *ball = (Ball *)ball.get_component(C_BALL);
  }
}


void reset_game_state()
{
  destroy_all_entities();

  create_initial_entities();

  game_state->restarting_game = false;
  game_state->simulate_game = true;
}


void restart_game()
{
  game_state->restarting_game = true;
  game_state->simulate_game = false;
}


bool should_restart_game()
{
  return game_state->restarting_game;
}

bool should_simulate_game()
{
  return game_state->simulate_game;
}

void init_game_state_system()
{
  game_state = (GameState *)malloc(sizeof(GameState));

  game_state->entity_pool = (EntityPool *)malloc(sizeof(EntityPool));
  game_state->entity_pool->num_entities = 0;
  game_state->entity_pool->entities = (Entity *)malloc(sizeof(Entity) * MAX_ENTITIES);

  game_state->restarting_game = true;
  game_state->simulate_game = false;
}


