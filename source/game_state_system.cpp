
// Interface
#include "game_state_system.h"
#include "entity.h"

// Needed components
#include "entity.h"
#include "component.h"

// Utilities
#include "my_math.h"
#include <stdlib.h> // malloc
#include <assert.h>

struct Entity
{
  const char *name = "";
  EntityID id = EntityID(0);

  bool alive = false;
  bool will_die = false;

  Component *components[NUM_COMPONENTS];
};


struct SerializedEntityHeader
{
  EntityID id = EntityID(0);

  unsigned active_components = 0;
  
  enum EntityFlags
  {
    ALIVE,
    WILL_DIE,
  };
  char flags = 0;
};

struct SerializedEntity
{
};



struct EntityPool
{
  static const int MAX_ENTITIES = 1024;

  int num_entities;
  Entity *entities;

  int num_entities_to_destroy;
  Entity **entities_to_destroy;
};

struct GameState
{
  EntityPool *entity_pool;

  bool will_restart_game;
  bool simulate_game;
};

struct SerializedGameStateHeader
{
  int num_entities;
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

Goal *EntityID::get_goal() const
{
  return (Goal *)game_state->entity_pool->entities[id].components[C_GOAL];
}

bool EntityID::operator==(const EntityID &other) const { return id == other.id; }

EntityID create_entity(const char *name)
{
  if(game_state->entity_pool->num_entities >= EntityPool::MAX_ENTITIES) assert(0);

  Entity entity;

  entity.name = name;
  for(int i = 0; i < NUM_COMPONENTS; i++) entity.components[i] = nullptr;

  entity.id = EntityID(game_state->entity_pool->num_entities);

  entity.alive = true;
  entity.will_die = false;

  game_state->entity_pool->entities[game_state->entity_pool->num_entities] = entity;
  game_state->entity_pool->num_entities++;
  return entity.id;
}

void EntityID::defer_destroy()
{
  game_state->entity_pool->entities[id].will_die = true;
}







static void destroy_entity(Entity *entity)
{
  entity->alive = false;
  entity->will_die = false;
  entity->name = "";
  entity->id = EntityID(0);

  for(int i = 0; i < NUM_COMPONENTS; i++)
  {
    if(entity->components[i] == nullptr) continue;
    destroy_component(entity->components[i]);
    entity->components[i] = nullptr;
  }
}

static void reset_all_entities()
{
  for(int i = 0; i < EntityPool::MAX_ENTITIES; i++)
  {
    if(game_state->entity_pool->entities[i].alive == false) continue;
    destroy_entity(&(game_state->entity_pool->entities[i]));
  }

  game_state->entity_pool->num_entities = 0;
  game_state->entity_pool->num_entities_to_destroy = 0;
}

void create_initial_entities()
{
  EntityID players[4];
  players[0] = create_entity("player 1");
  players[1] = create_entity("player 2");
  players[2] = create_entity("player 3");
  players[3] = create_entity("player 4");

  EntityID walls[6];
  walls[0] = create_entity("bottom left wall");
  walls[1] = create_entity("top left wall");
  walls[2] = create_entity("top wall");
  walls[3] = create_entity("top right wall");
  walls[4] = create_entity("bottom right wall");
  walls[5] = create_entity("bottom wall");

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

    //model->texture = "ball";

    //Ball *ball = (Ball *)ball.get_component(C_BALL);
  }
}


void set_game_to_restart()
{
  game_state->will_restart_game = true;
  game_state->simulate_game = false;
}


bool should_restart_game()
{
  return game_state->will_restart_game;
}

bool should_simulate_game()
{
  return game_state->simulate_game;
}

void reset_game_state()
{
  reset_all_entities();

  create_initial_entities();

  game_state->will_restart_game = false;
  game_state->simulate_game = true;
}

static int serialize_entity(Entity *entity, void *stream)
{
  char *stream_pos = (char *)stream;


  // Serialize header
  SerializedEntityHeader entity_header;
  entity_header.id = entity->id;

  unsigned active_components = 0;
  for(int i = 0; i < NUM_COMPONENTS; i++)
  {
    if(entity->components[i] != nullptr)
    {
      active_components |= 1 << i;
    }
  }
  entity_header.active_components = (short)active_components;

  entity_header.flags = 0;
  if(entity->alive) entity_header.flags |= 1 << SerializedEntityHeader::ALIVE;
  if(entity->will_die) entity_header.flags |= 1 << SerializedEntityHeader::WILL_DIE;

  *(SerializedEntityHeader *)stream_pos = entity_header;

  // Move stream forward
  stream_pos += sizeof(SerializedEntityHeader);


  // Serialize components
  for(int i = 0; i < NUM_COMPONENTS; i++)
  {
    if(entity->components[i] != nullptr)
    {
      int component_bytes = serialize_component(entity->components[i], stream_pos);
      stream_pos += component_bytes;
    }
  }

  int serialized_bytes = stream_pos - (char *)stream;
  return serialized_bytes;
}

static int deserialize_entity(const void *stream)
{
  char *stream_pos = (char *)stream;
  SerializedEntityHeader *serialized_header = (SerializedEntityHeader *)stream;

  EntityID id = serialized_header->id;
  bool alive = ((serialized_header->flags & (1 << SerializedEntityHeader::ALIVE)) != 0) ? true : false;
  bool will_die = ((serialized_header->flags & (1 << SerializedEntityHeader::WILL_DIE)) != 0) ? true : false;

  // Check if this entity already exists
  Entity *target_entity = &game_state->entity_pool->entities[id.id];
  if(game_state->entity_pool->entities[id.id].alive)
  {
    // Use existing entity

    // If this entity is dead or will die, remove it
    if(!alive || will_die)
    {
      destroy_entity(target_entity);

      // Early exit bceause we did not deserialize this
      return 0;
    }
  }
  else
  {
    // New entity created
    game_state->entity_pool->num_entities++;
  }


  target_entity->id = serialized_header->id;
  target_entity->alive = alive;
  target_entity->will_die = will_die;


  unsigned active_components = serialized_header->active_components;

  // Move the stream forward
  stream_pos += sizeof(SerializedEntityHeader);

  for(int i = 0; i < NUM_COMPONENTS; i++)
  {
    if(((1 << i) & active_components) == 0) continue;

    int component_bytes = deserialize_component((ComponentType)i, target_entity->id, stream_pos);
    stream_pos += component_bytes;
  }

  int serialized_bytes = stream_pos - (char *)stream;
  return serialized_bytes;
}

void serialize_game_state(void **out_stream, int *out_bytes)
{
  int num_entities = game_state->entity_pool->num_entities;
  char *stream = (char *)malloc(1400);
  char *stream_pos = stream;

  SerializedGameStateHeader game_state_header;
  game_state_header.num_entities = num_entities;

  *(SerializedGameStateHeader *)stream_pos = game_state_header;
  stream_pos += sizeof(SerializedGameStateHeader);

  for(int i = 0; i < num_entities; i++)
  {
    while(game_state->entity_pool->entities[i].alive == false) i++;

    Entity *entity = &(game_state->entity_pool->entities[i]);

    int bytes_written = serialize_entity(entity, stream_pos);
    stream_pos += bytes_written;
    assert(stream_pos < stream + 1400);
  }

  *out_stream = stream;
  int bytes = stream_pos - stream;
  *out_bytes = bytes;
}

void deserialize_game_state(const void *data, int bytes)
{
  char *end = (char *)data + bytes;
  char *stream_pos = (char *)data;

  // Read in all game states
  while(stream_pos < end)
  {
    SerializedGameStateHeader *game_state_header = (SerializedGameStateHeader *)stream_pos;
    stream_pos += sizeof(SerializedGameStateHeader);

    for(int i = 0; i < game_state_header->num_entities; i++)
    {
      int entity_size = deserialize_entity((const SerializedEntity *)stream_pos);
      stream_pos += entity_size;
    }

    // TODO: Save this game state
    // ...
  }
}

void init_game_state_system()
{
  game_state = (GameState *)malloc(sizeof(GameState));

  game_state->entity_pool = (EntityPool *)malloc(sizeof(EntityPool));
  game_state->entity_pool->num_entities = 0;
  game_state->entity_pool->entities = (Entity *)malloc(sizeof(Entity) * EntityPool::MAX_ENTITIES);
  game_state->entity_pool->num_entities_to_destroy = 0;
  game_state->entity_pool->entities_to_destroy = (Entity **)malloc(sizeof(Entity *) * EntityPool::MAX_ENTITIES);

  for(int i = 0; i < EntityPool::MAX_ENTITIES; i++) game_state->entity_pool->entities[i] = Entity();

  game_state->will_restart_game = true;
  game_state->simulate_game = false;
}


