
#include "entity.h"

#include "component.h"

#include <stdlib.h>
#include <assert.h>

struct Entity
{
  const char *name;

  ComponentHandle components[NUM_COMPONENTS];
};

static const int MAX_ENTITIES = 1024;
struct EntityData
{
  int num_entities;
  Entity *entities;
};

static EntityData *entity_data;

void init_entities()
{
  entity_data = (EntityData *)malloc(sizeof(EntityData));
  entity_data->num_entities = 0;
  entity_data->entities = (Entity *)malloc(sizeof(Entity) * MAX_ENTITIES);
}

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


void EntityID::attach_component(ComponentHandle component)
{
  component->enabled = true;
  component->parent = EntityID(id);
  entity_data->entities[id].components[component->component_type] = component;
}

ComponentHandle EntityID::get_component(ComponentType component_type) const
{
  return entity_data->entities[id].components[component_type];
}

