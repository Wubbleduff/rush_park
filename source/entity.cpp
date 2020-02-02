
#include "entity.h"

#include "component.h"

#include <stdlib.h>
#include <assert.h>

struct Entity
{
  const char *name;

  Component *components[NUM_COMPONENTS];
};

static const int MAX_ENTITIES = 1024;
struct EntityData
{
  int num_entities;
  Entity *entities;
};

static EntityData *entity_data;

Component *EntityID::add_component(ComponentType type)
{
  Component *component = create_component(EntityID(id), type);
  entity_data->entities[id].components[type] = component;
  return component;
}

void EntityID::remove_component(ComponentType type)
{
  Component *component_to_remove = entity_data->entities[id].components[type];
  destroy_component(component_to_remove);
  entity_data->entities[id].components[type] = nullptr;
}




Component *EntityID::get_component(ComponentType component_type) const
{
  return entity_data->entities[id].components[component_type];
}

Model *EntityID::get_model() const
{
  return (Model *)entity_data->entities[id].components[C_MODEL];
}

Ball *EntityID::get_ball() const
{
  return (Ball *)entity_data->entities[id].components[C_BALL];
}

Wall *EntityID::get_wall() const
{
  return (Wall *)entity_data->entities[id].components[C_WALL];
}

Player *EntityID::get_player() const
{
  return (Player *)entity_data->entities[id].components[C_PLAYER];
}




bool EntityID::operator==(const EntityID &other) const { return id == other.id; }

EntityID create_entity(const char *name)
{
  if(entity_data->num_entities >= MAX_ENTITIES) assert(0);

  Entity entity;

  entity.name = name;
  for(int i = 0; i < NUM_COMPONENTS; i++) entity.components[i] = nullptr;

  entity_data->entities[entity_data->num_entities] = entity;
  entity_data->num_entities++;
  return EntityID(entity_data->num_entities - 1);
}

#if 0
void destroy_entity(EntityID entity)
{
  Entity
}
#endif


void init_entities()
{
  entity_data = (EntityData *)malloc(sizeof(EntityData));
  entity_data->num_entities = 0;
  entity_data->entities = (Entity *)malloc(sizeof(Entity) * MAX_ENTITIES);
}

