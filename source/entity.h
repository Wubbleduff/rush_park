
#pragma once

enum ComponentType
{
  CMODEL,
  CANIMATION,
  CPHYSICS,
  CPLAYER,

  NUM_COMPONENTS
};

struct Entity
{
  const char *name;

  int components[NUM_COMPONENTS];
};

typedef int EntityHandle;

void init_entities();

EntityHandle create_entity(const char *name);

void add_component(EntityHandle entity, ComponentType component, int handle);
int get_component(EntityHandle entity, ComponentType component);

