
#include "player_collision_system.h"

#include "component.h"

#include "my_math.h"

#include <stdlib.h>
#include <assert.h>


// For resolution
struct CollisionInfo
{
  const ColliderComponent *a;
  const ColliderComponent *b;

  float depth;
  v2 normal;
};



static bool aabb_collision(const ColliderComponent *a_collider, const ColliderComponent *b_collider, CollisionInfo *info)
{
  assert(a_collider->type == ColliderComponent::RECT);
  assert(b_collider->type == ColliderComponent::RECT);

  ModelComponent *a_model = (ModelComponent *)a_collider->parent.get_component(C_MODEL);
  ModelComponent *b_model = (ModelComponent *)b_collider->parent.get_component(C_MODEL);

  v2 a_pos = a_model->position;
  v2 a_half_scale = a_collider->rect.scale / 2.0f;

  v2 b_pos = b_model->position;
  v2 b_half_scale = b_collider->rect.scale / 2.0f;

  float a_x_min = a_pos.x - a_half_scale.x;
  float a_x_max = a_pos.x + a_half_scale.x;
  float a_y_min = a_pos.y - a_half_scale.y;
  float a_y_max = a_pos.y + a_half_scale.y;

  float b_x_min = b_pos.x - b_half_scale.x;
  float b_x_max = b_pos.x + b_half_scale.x;
  float b_y_min = b_pos.y - b_half_scale.y;
  float b_y_max = b_pos.y + b_half_scale.y;


  // Early out
  if(a_x_min > b_x_max) return false;
  if(a_x_max < b_x_min) return false;
  if(a_y_min > b_y_max) return false;
  if(a_y_max < b_y_min) return false;




  float a_left_depth   = absf(b_x_max - a_x_min);
  float a_right_depth  = absf(b_x_min - a_x_max);
  float a_top_depth    = absf(b_y_min - a_y_max);
  float a_bottom_depth = absf(b_y_max - a_y_min);

  // Take the minimum penetration depth
  float depth = min(min(min(a_left_depth, a_right_depth), a_top_depth), a_bottom_depth);

  if(depth == a_left_depth)   info->normal = v2(-1.0f,  0.0f);
  if(depth == a_right_depth)  info->normal = v2( 1.0f,  0.0f);
  if(depth == a_top_depth)    info->normal = v2( 0.0f,  1.0f);
  if(depth == a_bottom_depth) info->normal = v2( 0.0f, -1.0f);



  info->a = a_collider;
  info->b = b_collider;
  info->depth = depth;

  return true;
}

void update_player_collision_system(float time_step)
{
  ComponentIterator<PlayerComponent> player_it = get_players_iterator();




  // Temporary
  int num_collisions = 0;
  static const int MAX_COLLISIONS = 32;
  CollisionInfo collisions[MAX_COLLISIONS];



  while(player_it != nullptr)
  {
    PlayerComponent *player_component = &(*player_it);
    const ColliderComponent *player_collider = (ColliderComponent *)player_component->parent.get_component(C_COLLIDER);

    ComponentIterator<ColliderComponent> collider_it = get_colliders_iterator();
    while(collider_it != nullptr)
    {
      const ColliderComponent *other_collider = &(*collider_it);
      if(player_collider == other_collider) { ++collider_it; continue; }


      CollisionInfo info = {};
      if(player_collider->type == ColliderComponent::RECT)
      {
        if(other_collider->type == ColliderComponent::RECT)
        {
          if(aabb_collision(player_collider, other_collider, &info))
          {
            collisions[num_collisions++] = info;
          }
        }

        if(other_collider->type == ColliderComponent::CIRCLE)
        {
          /*
          if(box_circle_collision(a, b, &info))
          {
            collisions[num_collisions++] = info;
          }
          */
        }
      }



      if(player_collider->type == ColliderComponent::CIRCLE)
      {
        if(other_collider->type == ColliderComponent::RECT)
        {
          /*
          if(box_circle_collision(b, a, &info))
          {
            collisions[num_collisions++] = info;
          }
          */
        }

        if(other_collider->type == ColliderComponent::CIRCLE)
        {
          /*
          if(circle_circle_collision(a, b, &info))
          {
            collisions[num_collisions++] = info;
          }
          */
        }
      }

      assert(num_collisions < MAX_COLLISIONS);
      ++collider_it;
    }

    ++player_it;
  }



  // Resolve collisions
  for(int i = 0; i < num_collisions; i++)
  {
    CollisionInfo *info = &(collisions[i]);

    ModelComponent *a_model = (ModelComponent *)info->a->parent.get_component(C_MODEL);
    ModelComponent *b_model  = (ModelComponent *)info->b->parent.get_component(C_MODEL);

    // So players won't collide with ghosty Razz
    {
      PlayerComponent *a_player = (PlayerComponent *)info->a->parent.get_component(C_PLAYER);
      PlayerComponent *b_player = (PlayerComponent *)info->b->parent.get_component(C_PLAYER);
      if(a_player && b_player)
      {
        if(a_player->character_type == PlayerComponent::RAZZ ||
           b_player->character_type == PlayerComponent::RAZZ)
        {
          continue;
        }
      }
    }


    float mass_sum = info->a->mass + info->b->mass;
    float a_move_amount = (info->b->mass / mass_sum) * (info->depth / 2.0f);
    float b_move_amount = (info->a->mass / mass_sum) * (info->depth / 2.0f);

    // player
    if(!info->a->is_static)
    {
      if(info->b->is_static)
      {
        a_model->position -= info->normal * info->depth;
      }
      else 
      {
        a_model->position -= info->normal * a_move_amount;
      }
    }

    // other
    if(!info->b->is_static)
    {
      if(info->a->is_static)
      {
        b_model->position += info->normal * info->depth;
      }
      else 
      {
        b_model->position += info->normal * b_move_amount;
      }
    }
  }

  // Move the players
  player_it = get_players_iterator();
  while(player_it != nullptr)
  {
    PlayerComponent *player_component = &(*player_it);
    ModelComponent *player_model = (ModelComponent *)player_component->parent.get_component(C_MODEL);

    player_model->position += player_component->velocity * time_step;

    ++player_it;
  }
}

