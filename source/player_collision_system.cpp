
#include "player_collision_system.h"

#include "component.h"

#include "my_math.h"

#include <stdlib.h>
#include <assert.h>


// For resolution
struct CollisionInfo
{
  Model *a;
  Model *b;

  float depth;
  v2 normal;
};


// Pass models for resolution later
static bool aabb_collision(const Rect *a, const Rect *b, Model *a_model, Model *b_model, CollisionInfo *info)
{
  v2 a_pos = a->position;
  v2 a_half_scale = a->scale / 2.0f;
  v2 b_pos = b->position;
  v2 b_half_scale = b->scale / 2.0f;

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



  info->a = a_model;
  info->b = b_model;
  info->depth = depth;

  return true;
}

void update_player_collision_system(float time_step)
{
  // Temporary
  int num_collisions = 0;
  static const int MAX_COLLISIONS = 32;
  CollisionInfo collisions[MAX_COLLISIONS];


  // Go through all players
  ComponentIterator<Player> players_it = get_players_iterator();
  while(players_it != nullptr)
  {
    Player *player = &(*players_it);
    Model *player_model = player->parent.get_model();

    Rect player_rect = Rect(player_model->position, player_model->scale);

    // Check against other players
    ComponentIterator<Player> other_players_it = ++players_it;
    while(other_players_it != nullptr)
    {
      Player *other_player = &(*other_players_it);
      Model *other_player_model = other_player->parent.get_model();
      Rect other_player_rect = Rect(other_player_model->position, other_player_model->scale);

      CollisionInfo info = {};
      if(aabb_collision(&player_rect, &other_player_rect, player_model, other_player_model, &info))
      {
        collisions[num_collisions++] = info;
      }

      assert(num_collisions < MAX_COLLISIONS);
      ++other_players_it;
    }

    // Check against walls
    ComponentIterator<Wall> walls_it = get_walls_iterator();
    while(walls_it != nullptr)
    {
      Wall *wall = &(*walls_it);
      Model *wall_model = wall->parent.get_model();
      Rect wall_rect = Rect(wall_model->position, wall_model->scale);

      CollisionInfo info = {};
      if(aabb_collision(&player_rect, &wall_rect, player_model, wall_model, &info))
      {
        collisions[num_collisions++] = info;
      }

      assert(num_collisions < MAX_COLLISIONS);
      ++walls_it;
    }

    ++players_it;
  }



  // Resolve collisions
  for(int i = 0; i < num_collisions; i++)
  {
    CollisionInfo *info = &(collisions[i]);

    Player *a_player = info->a->parent.get_player();
    assert(a_player);

    Player *b_player = info->b->parent.get_player();

    // So players won't collide with ghosty Razz
    {
      if(b_player)
      {
        if(a_player->character_type == Player::RAZZ ||
           b_player->character_type == Player::RAZZ)
        {
          continue;
        }
      }
    }

    // If b is also a player
    if(b_player)
    {
      float mass_sum = a_player->mass + b_player->mass;
      float a_move_amount = (b_player->mass / mass_sum) * (info->depth / 2.0f);
      float b_move_amount = (a_player->mass / mass_sum) * (info->depth / 2.0f);

      info->a->position -= info->normal * a_move_amount;
      info->b->position += info->normal * b_move_amount;
    }
    else if(info->b->parent.get_wall())
    {
      // b is a wall
      info->a->position -= info->normal * info->depth;
    }
  }

  // Move the players
  players_it = get_players_iterator();
  while(players_it != nullptr)
  {
    Player *player = &(*players_it);
    Model *player_model = (Model *)player->parent.get_component(C_MODEL);

    player_model->position += player->velocity * time_step;

    ++players_it;
  }
}

