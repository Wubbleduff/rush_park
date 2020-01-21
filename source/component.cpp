
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
    assert(count < capacity);

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



static ModelComponent *create_model_component(EntityID parent)
{
  ModelComponent model = {};

  model.component_type = C_MODEL;
  model.enabled = true;
  model.parent = parent;

  ComponentPool<ModelComponent> *pool = (ComponentPool<ModelComponent> *)component_collection->pools[C_MODEL];
  return pool->add(&model);
}

static ColliderComponent *create_collider_component(EntityID parent)
{
  ColliderComponent collider = {};

  collider.component_type = C_MODEL;
  collider.enabled = true;
  collider.parent = parent;

  ComponentPool<ColliderComponent> *pool = (ComponentPool<ColliderComponent> *)component_collection->pools[C_COLLIDER];
  return pool->add(&collider);
}

static BallComponent *create_ball_component(EntityID parent)
{
  BallComponent ball = {};

  ball.component_type = C_MODEL;
  ball.enabled = true;
  ball.parent = parent;

  ComponentPool<BallComponent> *pool = (ComponentPool<BallComponent> *)component_collection->pools[C_BALL];
  return pool->add(&ball);
}

static PlayerComponent *create_player_component(EntityID parent)
{
  PlayerComponent player = {};

  player.component_type = C_MODEL;
  player.enabled = true;
  player.parent = parent;

  ComponentPool<PlayerComponent> *pool = (ComponentPool<PlayerComponent> *)component_collection->pools[C_PLAYER];
  return pool->add(&player);
}

Component *create_component(EntityID parent, ComponentType type)
{
  switch(type)
  {
    case C_MODEL:    return create_model_component(parent);
    case C_COLLIDER: return create_collider_component(parent);
    case C_BALL:     return create_ball_component(parent);
    case C_PLAYER:   return create_player_component(parent);

    default: { assert(0); return nullptr; }
  }
}

void destroy_component(Component *component)
{
}

ComponentIterator<ModelComponent> get_models_iterator()
{
  ComponentPool<ModelComponent> *pool = (ComponentPool<ModelComponent> *)component_collection->pools[C_MODEL];
  if(pool->count == 0) return ComponentIterator<ModelComponent>(nullptr, 0);
  return ComponentIterator<ModelComponent>(pool->components, pool->count);
}
ComponentIterator<ColliderComponent> get_colliders_iterator()
{
  ComponentPool<ColliderComponent> *pool = (ComponentPool<ColliderComponent> *)component_collection->pools[C_COLLIDER];
  if(pool->count == 0) return ComponentIterator<ColliderComponent>(nullptr, 0);
  return ComponentIterator<ColliderComponent>(pool->components, pool->count);
}
ComponentIterator<BallComponent> get_balls_iterator()
{
  ComponentPool<BallComponent> *pool = (ComponentPool<BallComponent> *)component_collection->pools[C_BALL];
  if(pool->count == 0) return ComponentIterator<BallComponent>(nullptr, 0);
  return ComponentIterator<BallComponent>(pool->components, pool->count);
}
ComponentIterator<PlayerComponent> get_players_iterator()
{
  ComponentPool<PlayerComponent> *pool = (ComponentPool<PlayerComponent> *)component_collection->pools[C_PLAYER];
  if(pool->count == 0) return ComponentIterator<PlayerComponent>(nullptr, 0);
  return ComponentIterator<PlayerComponent>(pool->components, pool->count);
}




void init_component_collection()
{
  component_collection = new ComponentCollection();

  ComponentPool<ModelComponent> *model_pool = new ComponentPool<ModelComponent>();
  component_collection->pools[C_MODEL] = model_pool;
  model_pool->components = new ModelComponent[model_pool->capacity];
  model_pool->components_to_remove = new ModelComponent[model_pool->capacity];



  ComponentPool<ColliderComponent> *collider_pool = new ComponentPool<ColliderComponent>();
  component_collection->pools[C_COLLIDER] = collider_pool;
  collider_pool->components = new ColliderComponent[collider_pool->capacity];
  collider_pool->components_to_remove = new ColliderComponent[collider_pool->capacity];



  ComponentPool<BallComponent> *ball_pool = new ComponentPool<BallComponent>();
  component_collection->pools[C_BALL] = ball_pool;
  ball_pool->components = new BallComponent[ball_pool->capacity];
  ball_pool->components_to_remove = new BallComponent[ball_pool->capacity];



  ComponentPool<PlayerComponent> *player_pool = new ComponentPool<PlayerComponent>();
  component_collection->pools[C_PLAYER] = player_pool;
  player_pool->components = new PlayerComponent[player_pool->capacity];
  player_pool->components_to_remove = new PlayerComponent[player_pool->capacity];
}

