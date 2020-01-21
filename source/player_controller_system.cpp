
#include "player_controller_system.h"

#include "component.h"
#include "game_input.h"

#include <assert.h>



// Check if the player has hit any balls
static void check_ball_hits(PlayerComponent *player)
{
  ComponentIterator<BallComponent> ball_it = get_balls_iterator();

  ModelComponent *player_model = (ModelComponent *)player->parent.get_component(C_MODEL);

  // Loop through all balls
  int num_balls_hit = 0;;
  static const int MAX_NUM_BALLS_HIT = 4;
  EntityID balls_hit[MAX_NUM_BALLS_HIT];
  while(ball_it != nullptr)
  {
    BallComponent *ball = &(*ball_it);
    ModelComponent *ball_model = (ModelComponent *)ball->parent.get_component(C_MODEL);
    ColliderComponent *ball_collider = (ColliderComponent *)ball->parent.get_component(C_COLLIDER);

    // Check if the ball is in range of a hit

    v2 player_pos = player_model->position; 
    float player_radius = player_model->scale.y / 2.0f;

    v2 ball_pos = ball_model->position;
    float ball_radius = ball_collider->circle.radius;

    float r = player_radius + ball_radius;
    bool in_range = (length_squared(player_pos - ball_pos) <= r * r);
    if(in_range)
    {
      // If it is, add to a list of balls hit this frame
      assert(num_balls_hit <= MAX_NUM_BALLS_HIT);
      balls_hit[num_balls_hit++] = ball->parent;;
    }

    ++ball_it;
  }



  // Resolve ball hits
  for(int i = 0; i < num_balls_hit; i++)
  {
    // Make sure we resolve a hit on a ball only once per hit
    BallComponent *this_frame_ball = (BallComponent *)balls_hit[i].get_component(C_BALL);
    bool found_same_ball = false;
    for(int j = 0; j < player->num_balls_hit_last_frame; j++)
    {
      BallComponent *last_frame_ball = (BallComponent *)player->balls_hit_last_frame[i].get_component(C_BALL);
      if(this_frame_ball == last_frame_ball) { found_same_ball = true; break; }
    }

    if(!found_same_ball)
    {
      // Affect the ball
      v2 mouse_pos = mouse_world_position();
      v2 to_mouse = unit(mouse_pos - player_model->position);
      float hit_power = player->hit_power;
      this_frame_ball->velocity = v2(0.0f, 1.0f); //to_mouse;
      this_frame_ball->speed += hit_power;
    }
  }



  // Record hits for next frame
  player->num_balls_hit_last_frame = num_balls_hit;
  for(int i = 0; i < num_balls_hit; i++)
  {
    player->balls_hit_last_frame[i] = balls_hit[i];
  }
}

void update_player_controller_system(float time_step)
{
  ComponentIterator<PlayerComponent> player_it = get_players_iterator();

  int i = 0;
  while(player_it != nullptr)
  {
    PlayerComponent *player = &(*player_it);
    ModelComponent *player_model = (ModelComponent *)player->parent.get_component(C_MODEL);

    if(!player->enabled)       { ++player_it; continue; } 
    if(!player_model->enabled) { ++player_it; continue; } 

    if(i != 0) { ++player_it; continue; }

    v2 move_dir = analog_state(0, INPUT_MOVE);
    if(!(move_dir.x == 0.0f && move_dir.y == 0.0f)) move_dir = unit(move_dir);


    float dampener = 0.1f; // This is here so that players will set roughly 1 for "movement power" 
    player->velocity += move_dir * player->move_power * time_step * dampener;
    player->velocity -= player->velocity * player->drag * time_step;

    if(button_toggled_down(i, INPUT_SWING))
    {
      player->state = PlayerComponent::SWINGING;
      player->swing_timer = player->swing_time;
    }


    if(player->state == PlayerComponent::SWINGING)
    {
      player->swing_timer -= time_step;
      if(player->swing_timer <= 0.0f)
      {
        player->swing_timer = 0.0f;
        player->state = PlayerComponent::DEFAULT;
      }


      check_ball_hits(player);
    }
    else
    {
      player->num_balls_hit_last_frame = 0;
    }

    ++player_it;
    ++i;
  }
}

