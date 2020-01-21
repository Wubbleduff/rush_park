
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
};


struct ModelComponent : Component
{
  v2 position = v2();
  float depth = 0.0f;
  v2 scale = v2(1.0f, 1.0f);
  float rotation = 0.0f;

  Color color = Color(1.0f, 1.0f, 1.0f, 1.0f);

  const char *texture = nullptr;



  //void destroy();
};



struct ColliderComponent : public Component
{
  enum Type
  {
    CIRCLE,
    RECT,
  } type = RECT;

  struct Circle { float radius; };
  struct Rect   { v2 scale; };
  union
  {
    Circle circle;
    Rect rect;
  };

  bool is_static;
  float mass;



  //void destroy();

  ColliderComponent() {  }
};

struct BallComponent : public Component
{
  float drag = 0.001f;
  v2 velocity = v2();
  float speed = 0.0f;



  //void destroy();
};


struct PlayerComponent : public Component
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

  float move_power = 1.0f; // aka acceleration, time to speed up, etc
  float hit_power = 100.0f;

  v2 velocity = v2();
  float drag = 0.5f;

  float swing_time = 0.0f;
  float swing_timer = 0.0f;

  int num_balls_hit_last_frame = 0;
  static const int MAX_NUM_BALLS_HIT = 4;
  EntityID balls_hit_last_frame[MAX_NUM_BALLS_HIT];



  //void destroy();
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

ComponentIterator<ModelComponent> get_models_iterator();
ComponentIterator<ColliderComponent> get_colliders_iterator();
ComponentIterator<BallComponent> get_balls_iterator();
ComponentIterator<PlayerComponent> get_players_iterator();


void init_component_collection();



