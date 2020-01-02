
#include "entity.h"

#include <stdlib.h>
#include <assert.h>


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

EntityHandle create_entity(const char *name)
{
  if(entity_data->num_entities >= MAX_ENTITIES) assert(0);

  Entity entity;

  entity.name = name;
  for(int i = 0; i < NUM_COMPONENTS; i++) entity.components[i] = -1;

  entity_data->entities[entity_data->num_entities] = entity;
  entity_data->num_entities++;
  return entity_data->num_entities - 1;
}

void add_component(EntityHandle entity, ComponentType component, int handle)
{
  entity_data->entities[entity].components[component] = handle;
}

int get_component(EntityHandle entity, ComponentType component)
{
  return entity_data->entities[entity].components[component];
}

