
#pragma once

#include "component_types.h"
#include "entity.h"

#include "my_math.h"


struct Component
{
  ComponentType component_type;
  bool enabled;
  EntityID parent;

  virtual void destroy() {};

  virtual int serialize(void *stream) = 0;
  virtual int deserialize(const void *stream) = 0;
};


struct Model : Component
{
  v2 position = v2();
  float depth = 0.0f;
  v2 scale = v2(1.0f, 1.0f);
  float rotation = 0.0f;

  Color color = Color(1.0f, 1.0f, 1.0f, 1.0f);

  //const char *texture = nullptr;

  //void destroy();
  virtual int serialize(void *stream);
  virtual int deserialize(const void *stream);
};

struct Wall : public Component
{
  v2 scale;

  // void destroy();
  virtual int serialize(void *stream);
  virtual int deserialize(const void *stream);
};

struct Ball : public Component
{
  float drag = 2.0f;
  v2 velocity = v2();
  float hit_speed = 3.0f;
  float hit_speed_increment = 10.0f;



  //void destroy();
  virtual int serialize(void *stream);
  virtual int deserialize(const void *stream);
};


struct Player : public Component
{
  int num;

  enum CharacterType
  {
    CRANKY_S_PANKY,
    RAZZ,
    SCUM,
    TAG,
  } character_type = CRANKY_S_PANKY;

  enum State
  {
    DEFAULT,
    SWINGING,
  } state = DEFAULT;

  // Physics
  float move_power = 150.0f; // aka acceleration, time to speed up, etc
  v2 velocity = v2();
  float drag = 20.0f;
  float mass = 1.0f;

  // Smack
  float hit_radius = 1.5f;
  float arc_length = 0.25f;
  float hit_power = 100.0f;
  float swing_time = 0.2f;
  float swing_timer = 0.0f;
  v2 hit_direction = v2();

  static const int MAX_BALLS_HIT = 4;
  int num_balls_hit_last_frame = 0;
  EntityID balls_hit_last_frame[MAX_BALLS_HIT] = {};

  //void destroy();
  virtual int serialize(void *stream);
  virtual int deserialize(const void *stream);
};

struct Goal : public Component
{
  enum Team
  {
    GREEN,
    PINK
  } team;

  virtual int serialize(void *stream);
  virtual int deserialize(const void *stream);
};

template<typename T>
struct ComponentIterator
{
  T *component = nullptr;
  int num_left = 0;



  ComponentIterator(int num_left) : component(nullptr), num_left(num_left) {}
  ComponentIterator(T *pointer, int num_left) : component(pointer), num_left(num_left) {}

  ComponentIterator &operator++()
  {
    num_left--;
    if(num_left == 0) { component = nullptr; return *this; }

    do
    {
      component++;
    } while(!component->enabled);

    return *this;
  }

  ComponentIterator operator++(int) { ComponentIterator temp = *this; operator++(); return temp; }
  T *operator->() { return component;  }
  T &operator*()  { return *component; } 
  bool operator==(const ComponentIterator &other) { return component == other.component; }
  bool operator==(const T *other) { return component == other; }
  bool operator!=(const T *other) { return component != other; }
};


#if 0
ModelComponent *create_model_component(EntityID parent, v2 position = v2(), float depth = 0.0f, v2 scale = v2(1.0f, 1.0f), float rotation = 0.0f, Color color = Color(1.0f, 1.0f, 1.0f, 1.0f));
ColliderComponent *create_collider_component(EntityID parent, ColliderComponent::Type type = ColliderComponent::RECT, bool is_static = false);
BallComponent *create_ball_component(EntityID parent, float drag = 0.0f);
PlayerComponent *create_player_component(EntityID parent, int num, PlayerComponent::CharacterType type = PlayerComponent::CRANKY_S_PANKY);
#endif

Component *create_component(EntityID parent, ComponentType type);
void destroy_component(Component *component);

ComponentIterator<Model> get_models_iterator();
ComponentIterator<Wall> get_walls_iterator();
ComponentIterator<Ball> get_balls_iterator();
ComponentIterator<Player> get_players_iterator();
ComponentIterator<Goal> get_goals_iterator();

int serialize_component(Component *component, void *stream);
int deserialize_component(ComponentType type, EntityID parent, const void *stream);

void init_component_collection();



