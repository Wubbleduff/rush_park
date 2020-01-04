
#pragma once

#include "component_handle.h"
#include "entity.h"

#include "my_math.h"


struct Component
{
  ComponentType component_type;
  bool enabled = false;
  EntityID parent;
};


struct ModelComponent : Component
{
  v2 position;
  float depth;
  v2 scale;
  float rotation;


  Color color;
};


struct ColliderComponent : public Component
{
  enum Type
  {
    CIRCLE,
    RECT,
  } type;

  struct Circle { float radius; };
  struct Rect   { v2 scale; };
  union
  {
    Circle circle;
    Rect rect;
  };

  ColliderComponent() {}
};

struct KinematicsComponent : public Component
{
  float drag;
  v2 velocity;
};

struct PlayerComponent : public Component
{
  int num;

  float acceleration;

  enum Type
  {
    RAZZ,
    CRANKY_S_PANKY,
    TAG,
    SCUM,
  };
};


ModelHandle create_model_component(v2 position = v2(), float depth = 0.0f, v2 scale = v2(1.0f, 1.0f), float rotation = 0.0f, Color color = Color(1.0f, 1.0f, 1.0f, 1.0f));
ColliderHandle create_collider_component();

void init_component_pool();

void get_model_components_array(unsigned *out_num_model_components, ModelComponent **out_model_components);
void get_collider_components_array(unsigned *out_num_collider_components, ColliderComponent **out_collider_components);


