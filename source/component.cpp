
#include "component.h"


#include <stdlib.h>

struct ComponentPool
{
  static const int MAX_MODELS = 1024;
  unsigned num_models;
  ModelComponent *model_components;

  static const int MAX_COLLIDERS = 1024;
  unsigned num_colliders;
  ColliderComponent *collider_components;

  // ...
};

static ComponentPool *component_pool;



ModelHandle create_model_component(v2 position, float depth, v2 scale, float rotation, Color color)
{
  ModelComponent model = {};

  model.component_type = C_MODEL;

  model.position = position;
  model.depth = depth;
  model.scale = scale;
  model.rotation = rotation;
  model.color = color;

  component_pool->model_components[component_pool->num_models] = model;
  component_pool->num_models++;
  return &(component_pool->model_components[component_pool->num_models - 1]);
}

ColliderHandle create_collider_component()
{
  ColliderComponent collider = {};

  collider.component_type = C_COLLIDER;

  component_pool->collider_components[component_pool->num_colliders] = collider;
  component_pool->num_colliders++;
  return &(component_pool->collider_components[component_pool->num_colliders - 1]);
}


void get_model_components_array(unsigned *out_num_model_components, ModelComponent **out_model_components)
{
  *out_num_model_components = component_pool->num_models;
  *out_model_components = component_pool->model_components;
}

void get_collider_components_array(unsigned *out_num_collider_components, ColliderComponent **out_collider_components)
{
  *out_num_collider_components = component_pool->num_colliders;
  *out_collider_components = component_pool->collider_components;
}


void init_component_pool()
{
  component_pool = (ComponentPool *)malloc(sizeof(ComponentPool));

  component_pool->num_models = 0;
  component_pool->model_components = (ModelComponent *)malloc(sizeof(ModelComponent) * ComponentPool::MAX_MODELS);

  component_pool->num_colliders = 0;
  component_pool->collider_components = (ColliderComponent *)malloc(sizeof(ColliderComponent) * ComponentPool::MAX_COLLIDERS);
}

