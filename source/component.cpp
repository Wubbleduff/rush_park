
#include "component.h"


#include <stdlib.h>
#include <assert.h>


template<typename T>
static T *find_slot_in_pool(T *pool, unsigned count)
{
  // Look for a free slot
  for(unsigned i = 0; i < count; i++)
  {
    if(pool[i].parent == EntityID(NO_ENTITY_ID))
    {
      return &(pool[i]);
    }
  }

  // Use the end
  return &(pool[count]);
}

static void remove_all_deferred_components(Component *pool, unsigned num_to_remove)
{
  for(unsigned i = 0; i < num_to_remove; i++)
  {
    pool[i].parent = EntityID(NO_ENTITY_ID);
    pool[i].enabled = false;
  }
}


struct BaseComponentPool
{
  unsigned count;

  unsigned num_to_remove;
  Component *components_to_remove;


  //virtual Component *add(const Component *component);
  //virtual void defer_remove(const Component *component_to_remove);

  virtual void remove_all_deferred()
  {
    remove_all_deferred_components(components_to_remove, num_to_remove);
    count -= num_to_remove;
    num_to_remove = 0;
  };
};



template<typename T>
struct ComponentPool : public BaseComponentPool
{
  const int capacity = 1024;
  T *components;

  T *add(const T *in_data)
  {
    // Make sure we have space
    assert((int)count < capacity);

    T *slot = (T *)find_slot_in_pool(components, count);
    *slot = *in_data;
    count++;

    return slot;
  }

  void defer_remove(const T *component_to_remove)
  {
    components_to_remove[num_to_remove++] = component_to_remove;
  }
};




struct ComponentCollection
{
  BaseComponentPool *pools[NUM_COMPONENTS];
};

static ComponentCollection *component_collection;



static Model *create_model_component(EntityID parent)
{
  Model model = {};

  model.component_type = C_MODEL;
  model.enabled = true;
  model.parent = parent;

  ComponentPool<Model> *pool = (ComponentPool<Model> *)component_collection->pools[C_MODEL];
  return pool->add(&model);
}

static Wall *create_wall_component(EntityID parent)
{
  Wall wall = {};

  wall.component_type = C_WALL;
  wall.enabled = true;
  wall.parent = parent;

  ComponentPool<Wall> *pool = (ComponentPool<Wall> *)component_collection->pools[C_WALL];
  return pool->add(&wall);
}

static Ball *create_ball_component(EntityID parent)
{
  Ball ball = {};

  ball.component_type = C_BALL;
  ball.enabled = true;
  ball.parent = parent;

  ComponentPool<Ball> *pool = (ComponentPool<Ball> *)component_collection->pools[C_BALL];
  return pool->add(&ball);
}

static Player *create_player_component(EntityID parent)
{
  Player player = {};

  player.component_type = C_PLAYER;
  player.enabled = true;
  player.parent = parent;

  ComponentPool<Player> *pool = (ComponentPool<Player> *)component_collection->pools[C_PLAYER];
  return pool->add(&player);
}

static Goal *create_goal_component(EntityID parent)
{
  Goal goal = {};

  goal.component_type = C_GOAL;
  goal.enabled = true;
  goal.parent = parent;

  ComponentPool<Goal> *pool = (ComponentPool<Goal> *)component_collection->pools[C_GOAL];
  return pool->add(&goal);
}

Component *create_component(EntityID parent, ComponentType type)
{
  switch(type)
  {
    case C_MODEL:    return create_model_component(parent);
    case C_WALL:     return create_wall_component(parent);
    case C_BALL:     return create_ball_component(parent);
    case C_PLAYER:   return create_player_component(parent);
    case C_GOAL:     return create_goal_component(parent);

    default: { assert(0); return nullptr; }
  }
}

void destroy_component(Component *component)
{
  component->parent = EntityID(-1);
  component->enabled = false;

  switch(component->component_type)
  {
    case C_MODEL:  { (ComponentPool<Model> *)component_collection->pools[C_MODEL]->count--; break; }
    case C_WALL:   { (ComponentPool<Wall> *)component_collection->pools[C_WALL]->count--; break; }
    case C_BALL:   { (ComponentPool<Ball> *)component_collection->pools[C_BALL]->count--; break; }
    case C_PLAYER: { (ComponentPool<Player> *)component_collection->pools[C_PLAYER]->count--; break; }
    case C_GOAL:   { (ComponentPool<Goal> *)component_collection->pools[C_GOAL]->count--; break; }
  }
}

ComponentIterator<Model> get_models_iterator()
{
  ComponentPool<Model> *pool = (ComponentPool<Model> *)component_collection->pools[C_MODEL];
  if(pool->count == 0) return ComponentIterator<Model>(nullptr, 0);
  return ComponentIterator<Model>(pool->components, pool->count);
}
ComponentIterator<Wall> get_walls_iterator()
{
  ComponentPool<Wall> *pool = (ComponentPool<Wall> *)component_collection->pools[C_WALL];
  if(pool->count == 0) return ComponentIterator<Wall>(nullptr, 0);
  return ComponentIterator<Wall>(pool->components, pool->count);
}
ComponentIterator<Ball> get_balls_iterator()
{
  ComponentPool<Ball> *pool = (ComponentPool<Ball> *)component_collection->pools[C_BALL];
  if(pool->count == 0) return ComponentIterator<Ball>(nullptr, 0);
  return ComponentIterator<Ball>(pool->components, pool->count);
}
ComponentIterator<Player> get_players_iterator()
{
  ComponentPool<Player> *pool = (ComponentPool<Player> *)component_collection->pools[C_PLAYER];
  if(pool->count == 0) return ComponentIterator<Player>(nullptr, 0);
  return ComponentIterator<Player>(pool->components, pool->count);
}
ComponentIterator<Goal> get_goals_iterator()
{
  ComponentPool<Goal> *pool = (ComponentPool<Goal> *)component_collection->pools[C_GOAL];
  if(pool->count == 0) return ComponentIterator<Goal>(nullptr, 0);
  return ComponentIterator<Goal>(pool->components, pool->count);
}



static int write_int(void **stream, int a)
{
  **(int **)stream = a;
  *stream = (char *)(*stream) + sizeof(a);
  return sizeof(a);
}
static int write_float(void **stream, float a)
{
  **(float **)stream = a;
  *stream = (char *)(*stream) + sizeof(a);
  return sizeof(a);
}
static int write_v2(void **stream, v2 a)
{
  **(v2 **)stream = a;
  *stream = (char *)(*stream) + sizeof(a);
  return sizeof(a);
}
static int write_color(void **stream, Color a)
{
  **(Color **)stream = a;
  *stream = (char *)(*stream) + sizeof(a);
  return sizeof(a);
}
static int read_int(void const **stream)
{
  int a = **(int **)stream;
  *stream = (char *)(*stream) + sizeof(a);
  return a;
}
static float read_float(void const **stream)
{
  float a = **(float **)stream;
  *stream = (char *)(*stream) + sizeof(a);
  return a;
}
static v2 read_v2(void const **stream)
{
  v2 a = **(v2 **)stream;
  *stream = (char *)(*stream) + sizeof(a);
  return a;
}
static Color read_color(void const **stream)
{
  Color a = **(Color **)stream;
  *stream = (char *)(*stream) + sizeof(a);
  return a;
}


int Model::serialize(void *stream)
{
  int bytes = 0;
  bytes += write_v2(&stream, position);
  bytes += write_float(&stream, depth);
  bytes += write_v2(&stream, scale);
  bytes += write_float(&stream, rotation);
  bytes += write_color(&stream, color);
  return bytes;
}
int Model::deserialize(const void *stream)
{
  position = read_v2(&stream);
  depth = read_float(&stream);
  scale = read_v2(&stream);
  rotation = read_float(&stream);
  color = read_color(&stream);
  int bytes = sizeof(position) + sizeof(depth) + sizeof(scale) + sizeof(rotation) + sizeof(color);
  return bytes;
}

int Wall::serialize(void *stream)
{
  int bytes = 0;
  bytes += write_v2(&stream, scale);
  return bytes;
}
int Wall::deserialize(const void *stream)
{
  scale = read_v2(&stream);
  int bytes = sizeof(scale);
  return bytes;
}

int Ball::serialize(void *stream)
{
  int bytes = 0;
  bytes += write_float(&stream, drag);
  bytes += write_v2(&stream, velocity);
  bytes += write_float(&stream, hit_speed);
  bytes += write_float(&stream, hit_speed_increment);
  return bytes;
}
int Ball::deserialize(const void *stream)
{
  drag = read_float(&stream);
  velocity = read_v2(&stream);
  hit_speed = read_float(&stream);
  hit_speed_increment = read_float(&stream);
  int bytes = sizeof(drag) + sizeof(velocity) + sizeof(hit_speed) + sizeof(hit_speed_increment);
  return bytes;
}

int Player::serialize(void *stream)
{
  int bytes = 0;

  bytes += write_float(&stream, move_power);
  bytes += write_v2(&stream, velocity);
  bytes += write_float(&stream, drag);
  bytes += write_float(&stream, mass);
  bytes += write_float(&stream, hit_radius);
  bytes += write_float(&stream, arc_length);
  bytes += write_float(&stream, hit_power);
  bytes += write_float(&stream, swing_time);
  bytes += write_float(&stream, swing_timer);
  bytes += write_v2(&stream, hit_direction);
  //bytes += write_int(&stream, );
  //bytes += write_int(&stream, );
  //bytes += write_int(&stream, );
  //bytes += write_int(&stream, );
  //bytes += write_int(&stream, );


  return bytes;
}
int Player::deserialize(const void *stream)
{
  move_power = read_float(&stream);
  velocity = read_v2(&stream);
  drag = read_float(&stream);
  mass = read_float(&stream);
  hit_radius = read_float(&stream);
  arc_length = read_float(&stream);
  hit_power = read_float(&stream);
  swing_time = read_float(&stream);
  swing_timer = read_float(&stream);
  hit_direction = read_v2(&stream);
  int bytes = sizeof(float) * 8 + sizeof(v2) * 2;
  return bytes;
}

int Goal::serialize(void *stream)
{
  int bytes = 0;
  bytes += write_int(&stream, team);
  return bytes;
}
int Goal::deserialize(const void *stream)
{
  team = (Goal::Team)read_int(&stream);
  int bytes = sizeof(int);
  return bytes;
}



int serialize_component(Component *component, void *stream)
{
  return component->serialize(stream);
}

int deserialize_component(ComponentType type, EntityID parent, const void *stream)
{
  Component *target_component = nullptr;

  // Check if the parent already has this component
  if(parent.get_component(type) == nullptr)
  {
    parent.add_component(type);
  }
  target_component = parent.get_component(type);

  switch(type)
  {
    case C_MODEL:  { return parent.get_model()->deserialize(stream); }
    case C_WALL:   { return parent.get_wall()->deserialize(stream); }
    case C_BALL:   { return parent.get_ball()->deserialize(stream); }
    case C_PLAYER: { return parent.get_player()->deserialize(stream); }
    case C_GOAL:   { return parent.get_goal()->deserialize(stream); }
  }

  return 0;
}




void init_component_collection()
{
  component_collection = new ComponentCollection();

  ComponentPool<Model> *model_pool = new ComponentPool<Model>();
  component_collection->pools[C_MODEL] = model_pool;
  model_pool->components = new Model[model_pool->capacity];
  model_pool->components_to_remove = new Model[model_pool->capacity];



  ComponentPool<Wall> *wall_pool = new ComponentPool<Wall>();
  component_collection->pools[C_WALL] = wall_pool;
  wall_pool->components = new Wall[wall_pool->capacity];
  wall_pool->components_to_remove = new Wall[wall_pool->capacity];



  ComponentPool<Ball> *ball_pool = new ComponentPool<Ball>();
  component_collection->pools[C_BALL] = ball_pool;
  ball_pool->components = new Ball[ball_pool->capacity];
  ball_pool->components_to_remove = new Ball[ball_pool->capacity];



  ComponentPool<Player> *player_pool = new ComponentPool<Player>();
  component_collection->pools[C_PLAYER] = player_pool;
  player_pool->components = new Player[player_pool->capacity];
  player_pool->components_to_remove = new Player[player_pool->capacity];



  ComponentPool<Goal> *goal_pool = new ComponentPool<Goal>();
  component_collection->pools[C_GOAL] = goal_pool;
  goal_pool->components = new Goal[goal_pool->capacity];
  goal_pool->components_to_remove = new Goal[goal_pool->capacity];
}

