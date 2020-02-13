
#pragma once

#include "component_types.h"

static const unsigned NO_ENTITY_ID = (unsigned)(-1);

struct Component;
struct Model;
struct Wall;
struct Ball;
struct Player;

struct EntityID
{
  unsigned id;

  Component *add_component(ComponentType type);
  void remove_component(ComponentType type);
  Component *get_component(ComponentType type) const;

  Model *get_model() const;
  Wall *get_wall() const;
  Ball *get_ball() const;
  Player *get_player() const;

  bool EntityID::operator==(const EntityID &other) const;

  EntityID() {}
  explicit EntityID(unsigned id) : id(id) {}
};


EntityID create_entity(const char *name);
void destroy(EntityID entity);

