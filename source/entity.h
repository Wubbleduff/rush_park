
#pragma once

#include "component_handle.h"

struct EntityID
{
  unsigned id;

  void attach_component(ComponentHandle component);
  ComponentHandle get_component(ComponentType type) const;

  EntityID() {}
  explicit EntityID(unsigned id) : id(id) {}
};


EntityID create_entity(const char *name);

void init_entities();

