
#include "collision_system.h"

#include "component.h"

#include <stdlib.h>
#include <assert.h>



static bool box_box_collision(const ColliderComponent *a_collider, const ColliderComponent *b_collider)
{
  ModelHandle a_model = (ModelHandle)a_collider->parent.get_component(C_MODEL);
  ModelHandle b_model = (ModelHandle)b_collider->parent.get_component(C_MODEL);

  v2 a_pos = a_model->position;
  v2 a_scale = a_model->scale;

  v2 b_pos = b_model->position;
  v2 b_scale = b_model->scale;

  float a_x_min = a_pos.x - a_scale.x / 2.0f;
  float a_x_max = a_pos.x + a_scale.x / 2.0f;
  float a_y_min = a_pos.y - a_scale.y / 2.0f;
  float a_y_max = a_pos.y + a_scale.y / 2.0f;

  float b_x_min = b_pos.x - b_scale.x / 2.0f;
  float b_x_max = b_pos.x + b_scale.x / 2.0f;
  float b_y_min = b_pos.y - b_scale.y / 2.0f;
  float b_y_max = b_pos.y + b_scale.y / 2.0f;


  if(a_x_min > b_x_max) return false;
  if(a_x_max < b_x_min) return false;
  if(a_y_min > b_y_max) return false;
  if(a_y_max < b_y_min) return false;

  return true;
}

void update_collision_system(float time_step)
{
  // Components to process:
  // ColliderComponent : read
  // ModelComponent : read / write

  unsigned num_collider_components;
  ColliderComponent *collider_components;
  get_collider_components_array(&num_collider_components, &collider_components);


  for(int i = 0; i < num_collider_components; i++)
  {
    const ColliderComponent *a = &(collider_components[i]);
    if(!a->enabled) continue;

    for(int j = i + 1; j < num_collider_components; j++)
    {
      const ColliderComponent *b = &(collider_components[j]);
      if(!b->enabled) continue;


      ModelHandle player_model = (ModelHandle)a->parent.get_component(C_MODEL);

      if(box_box_collision(a, b))
      {
        player_model->color = Color(0, 1, 1, 1);
      }
      else
      {
        player_model->color = Color(0, 0, 1, 1);
      }
    }
  }
}

