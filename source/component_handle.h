
#pragma once

enum ComponentType
{
  C_NO_TYPE = -1,

  C_MODEL = 0,
  C_COLLIDER,
  C_KINEMATICS,
  C_PLAYER,

  NUM_COMPONENTS
};


struct Component;
typedef Component *ComponentHandle;

struct ModelComponent;
typedef ModelComponent *ModelHandle;

struct ColliderComponent;
typedef ColliderComponent *ColliderHandle;


