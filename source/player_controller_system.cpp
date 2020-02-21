
#include "player_controller_system.h"

#include "component.h"
#include "game_input.h"

#include "rendering_system.h"

#include <assert.h>



// Check if the player has hit any balls
#if 0
static void check_ball_hits(Player *player)
{
  ComponentIterator<BallComponent> ball_it = get_balls_iterator();

  Model *player_model = (Model *)player->parent.get_component(C_MODEL);

  // Loop through all balls
  int num_balls_hit = 0;;
  static const int MAX_NUM_BALLS_HIT = 4;
  EntityID balls_hit[MAX_NUM_BALLS_HIT];
  while(ball_it != nullptr)
  {
    BallComponent *ball = &(*ball_it);
    Model *ball_model = (Model *)ball->parent.get_component(C_MODEL);
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
#endif

void update_player_controller_system(float time_step)
{
  ComponentIterator<Player> player_it = get_players_iterator();

  int i = 0;
  while(player_it != nullptr)
  {
    Player *player = &(*player_it);
    Model *player_model = (Model *)player->parent.get_component(C_MODEL);

    if(!player->enabled)       { ++player_it; continue; } 
    if(!player_model->enabled) { ++player_it; continue; } 

    if(i != 0) { ++player_it; ++i; continue; }

    v2 move_dir = stick_state(0, INPUT_MOVE);
    if(!(move_dir.x == 0.0f && move_dir.y == 0.0f)) move_dir = unit(move_dir);

    v2 player_to_mouse = mouse_world_position() - player_model->position;
    if(length_squared(player_to_mouse) != 0.0f) player->hit_direction = unit(player_to_mouse);
    /*
    v2 aim_dir = stick_state(0, INPUT_AIM);
    player->hit_direction = aim_dir;
    */


    player->velocity += move_dir * player->move_power * time_step;
    player->velocity -= player->velocity * player->drag * time_step;

    if(button_toggled_down(i, INPUT_SWING))
    {
      player->state = Player::SWINGING;
      player->swing_timer = player->swing_time;
    }


    if(player->state == Player::SWINGING)
    {
      player->swing_timer -= time_step;
      if(player->swing_timer <= 0.0f)
      {
        player->swing_timer = 0.0f;
        player->state = Player::DEFAULT;
      }

      player_model->color = Color(1.0f, 0.0f, 0.0f);

      debug_draw_circle(player_model->position, player->hit_radius, Color(1.0f, 1.0f, 0.0f, 0.25f), DRAW_FILLED);
    }
    else
    {
      player_model->color = Color(0.0f, 0.0f, 1.0f);

      player->num_balls_hit_last_frame = 0;
    }


    ++player_it;
    ++i;
  }
}

