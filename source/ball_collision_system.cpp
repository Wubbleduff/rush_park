
#include "ball_collision_system.h"
#include "component.h"

#include "rendering_system.h" // Debug draw
#include "game_input.h"
#include "imgui.h"

#include <assert.h>

struct Line
{
  v2 a, b;
  Line() { }
  Line(v2 a, v2 b) : a(a), b(b) { }
};

struct Circle
{
  v2 position;
  float radius;
  Circle() { }
  Circle(v2 pos, float radius) : position(pos), radius(radius) { }
};

struct Ghost
{
  v2 new_velocity = v2();
  v2 new_position = v2();
  float travel_t = INFINITY; // From the previous ghost
  bool done = false;
  bool goal = false;

  Player *hit_player = nullptr;
};






static const int MAX_GHOST_COLLISIONS = 32;
static int num_ghost_collisions = 0;
static Ghost ghost_collisions[MAX_GHOST_COLLISIONS] = {};

const static float PAD_EPSILON = 0.01f;

static bool debug_draw_ball_collisions = false;






static bool line_circle_collision(Line line, Circle circle, float *dist_t, v2 *normal)
{
  // Source: https://stackoverflow.com/questions/1073336/circle-line-segment-collision-detection-algorithm
  // Thank you bobobobo

  v2 d = line.b - line.a;
  v2 f = line.a - circle.position;

  float r = circle.radius;
  float a = dot(d, d);
  float b = 2.0f * dot(d, f);
  float c = dot(f, f) - r * r ;
  float discriminant = b*b-4.0f*a*c;
  if(discriminant < 0.0f)
  {
    return false;
  }
  else
  {
    discriminant = sqrtf(discriminant);

    // either solution may be on or off the ray so need to test both
    // t1 is always the smaller value, because BOTH discriminant and
    // a are nonnegative.
    float t1 = (-b - discriminant) / (2.0f*a);
    float t2 = (-b + discriminant) / (2.0f*a);

    // 3x HIT cases:
    //          -o->             --|-->  |            |  --|->
    // Impale(t1 hit,t2 hit), Poke(t1 hit,t2>1), ExitWound(t1<0, t2 hit), 


    if(t1 >= 0.0f && t1 <= 1.0f)
    {
      // t1 is the intersection, and it's closer than t2
      // (since t1 uses -b - discriminant)
      // Impale, Poke
      
      *dist_t = t1;
      v2 x_point = line.a + d * *dist_t;
      *normal = unit(x_point - circle.position);
      return true;
    }

    if(t2 >= 0.0f && t2 <= 1.0f)
    {
      // ExitWound
      //*dist_t = t2;
      //v2 x_point = line.a + d * t1;
      //*normal = x_point - circle.position;
      return false;
    }

    // no intersection: FallShort, Past, CompletelyInside
    return false;
  }
}

// dist_t returns the distance percentage along a
// normal returns the normal to b
static bool line_line_collision(Line a, Line b, float *dist_t, v2 *normal)
{
  v2 a_n = unit(find_ccw_normal(a.b - a.a));
  v2 b_n = unit(find_ccw_normal(b.b - b.a));

  if(dot(b.a - a.a, a_n) * dot(b.b - a.a, a_n) >= 0.0f) return false; // b crosses a
  if(dot(a.a - b.a, b_n) * dot(a.b - b.a, b_n) >= 0.0f) return false; // a crosses b
  if(dot(a.b - a.a, find_ccw_normal(b.b - b.a)) == 0.0f) return false; // collinear
  
  *dist_t = 1.0f - ( (a.b * b_n - b.a * b_n) / (a.b * b_n - a.a * b_n) );
  *normal = b_n;

  return true;
}

static bool circle_circle_collision(Circle a, Circle b)
{
  v2 diff = a.position - b.position;
  float radius_sum = (a.radius + b.radius) * (a.radius + b.radius);
  if(length_squared(diff) <= radius_sum) return true;
  return false;
}

static bool sweep_ball_against_rect(v2 ball_position, v2 ball_velocity, float ball_radius, Rect rect,
                                    float *out_dist_t, v2 *out_normal)
{
  // Create the ball's line
  Line travel_line = Line(ball_position, ball_position + ball_velocity);

  // Sweep circle against other rectangle

  // Create the corners and edges
  Circle corners[4];
  Line edges[4];
  {
    v2 rect_pos = rect.position;
    v2 half_scale = rect.scale / 2.0f;

    v2 rect_points[4];
    rect_points[0] = rect_pos + v2(-half_scale.x, -half_scale.y); // Bottom left
    rect_points[1] = rect_pos + v2( half_scale.x, -half_scale.y); // Bottom right
    rect_points[2] = rect_pos + v2( half_scale.x,  half_scale.y); // Top right
    rect_points[3] = rect_pos + v2(-half_scale.x,  half_scale.y); // Top left

    // Corners
    for(int i = 0; i < 4; i++)
    {
      corners[i] = Circle(rect_points[i], ball_radius);
    }
    // Edges
    for(int i = 0; i < 4; i++)
    {
      v2 p0 = rect_points[i];
      v2 p1 = rect_points[(i + 1) % 4];
      v2 n = unit(find_ccw_normal(p1 - p0));
      edges[i] = Line(p0 + n * ball_radius, p1 + n * ball_radius);
    }
  }

  // Debug draw
  if(debug_draw_ball_collisions)
  {
    for(int i = 0; i < 4; i++)
    {
      debug_draw_line(edges[i].a, edges[i].b, Color(0.0f, 0.5f, 0.0));
      debug_draw_circle(corners[i].position, corners[i].radius, Color(0.0f, 0.5f, 0.0));
    }
  }


  // Check the ball's line against the rounded rectangle
  bool hit_anything = false;
  float closest_dist_t = INFINITY;
  v2 closest_normal;

  // Check against all corners
  for(int i = 0; i < 4; i++)
  {
    float dist;
    v2 normal;
    bool hit = line_circle_collision(travel_line, corners[i], &dist, &normal);
    if(hit && dist < closest_dist_t)
    {
      hit_anything = true;
      closest_dist_t = dist;
      closest_normal = normal;
    }
  }

  // Check against all edges
  for(int i = 0; i < 4; i++)
  {
    float dist;
    v2 normal;
    bool hit = line_line_collision(travel_line, edges[i], &dist, &normal);
    if(hit && dist < closest_dist_t)
    {
      hit_anything = true;
      closest_dist_t = dist;
      closest_normal = normal;
    }
  }

  *out_dist_t = closest_dist_t;
  *out_normal = closest_normal;
  if(hit_anything) return true;
  return false;
}

static bool ball_hit_player_last_frame(Ball *ball, Player *player)
{
  for(int i = 0; i < player->num_balls_hit_last_frame; i++)
  {
    if(player->balls_hit_last_frame[i] == ball->parent)
    {
      return true;
    }
  }

  return false;
}

static void check_against_player_hits(Ball *ball, v2 ball_position, v2 ball_velocity, float ball_radius,
                                      float *closest_dist_t, Player **closest_player)
{
  *closest_dist_t = 1.0f;
  *closest_player = nullptr;

  ComponentIterator<Player> player_it = get_players_iterator();
  while(player_it != nullptr)
  {
    // Make sure the player's hitting
    if(!player_it->state == Player::SWINGING)
    {
      ++player_it;
      continue;
    }

    // Make sure you only hit smacks that haven't hit this ball yet
    if(ball_hit_player_last_frame(ball, &(*player_it)))
    {
      ++player_it;
      continue;
    }

    Model *player_model = (Model *)player_it->parent.get_component(C_MODEL);

    if(debug_draw_ball_collisions) debug_draw_circle(player_model->position, ball_radius + player_it->hit_radius);

    // TODO: Also check the swing arc

    // Check if the ball is right on top of the player
    if(circle_circle_collision(Circle(ball_position, ball_radius), Circle(player_model->position, player_it->hit_radius)))
    {
      *closest_dist_t = 0.0f;
      *closest_player = &(*player_it);
      return;
    }

    // Sweep ball travel line against the player's hit
    Circle ball_player_sum_circle = Circle(player_model->position, ball_radius + player_it->hit_radius);
    Line travel_line = Line(ball_position, ball_position + ball_velocity);

    float t = 0.0f;
    v2 n = v2();
    bool hit = line_circle_collision(travel_line, ball_player_sum_circle, &t, &n);
    if(hit && t < *closest_dist_t)
    {
      *closest_dist_t = t;
      *closest_player = &(*player_it);
    }

    ++player_it;
  }
}

static bool check_against_walls(v2 ball_position, v2 ball_velocity, float ball_radius, float *out_dist_t, v2 *out_normal)
{
  bool result = false;
  *out_dist_t = INFINITY;

  // Go through walls
  ComponentIterator<Wall> wall_it = get_walls_iterator();
  while(wall_it != nullptr)
  {
    Wall *wall = &(*wall_it);
    Model *wall_model = (Model *)wall->parent.get_component(C_MODEL);
    
    Rect wall_rect = Rect(wall_model->position, wall_model->scale);

    float dist_t;
    v2 n;
    bool hit = sweep_ball_against_rect(ball_position, ball_velocity, ball_radius, wall_rect, &dist_t, &n);

    // Add the final collision info
    if(hit && dist_t < *out_dist_t)
    {
      result = true;
      *out_dist_t = dist_t;
      *out_normal = n;
    }

    ++wall_it;
  }

  return result;
}

static bool check_against_goals(v2 ball_position, v2 ball_velocity, float ball_radius, float *out_dist_t, v2 *out_normal)
{
  bool result = false;
  *out_dist_t = INFINITY;

  // Go through goals
  ComponentIterator<Goal> goal_it = get_goals_iterator();
  while(goal_it != nullptr)
  {
    Goal *goal = &(*goal_it);
    Model *goal_model = (Model *)goal->parent.get_component(C_MODEL);
    
    Rect goal_rect = Rect(goal_model->position, goal_model->scale);

    float dist_t;
    v2 n;
    bool hit = sweep_ball_against_rect(ball_position, ball_velocity, ball_radius, goal_rect, &dist_t, &n);

    // Add the final collision info
    if(hit && dist_t < *out_dist_t)
    {
      result = true;
      *out_dist_t = dist_t;
      *out_normal = n;
    }

    ++goal_it;
  }

  return result;
}

static void add_ghost(Ghost *ghost)
{
  assert(num_ghost_collisions < MAX_GHOST_COLLISIONS);
  ghost_collisions[num_ghost_collisions++] = *ghost;
}

void update_ball_collision_system(float time_step)
{
  // For each ball
  //   while will not collide
  //     sweep circle along travel path and get collision info
  //     if no collisions: break
  //     pick "earliest" collision
  //     decide how to resolve the collision based on what it collided with
  //     resolve
  //

  ComponentIterator<Ball> ball_it = get_balls_iterator();

  // For each ball
  while(ball_it != nullptr)
  {
    Ball *ball = &(*ball_it);
    Model *ball_model = (Model *)ball->parent.get_component(C_MODEL);

    Ghost latest_ghost = {};
    latest_ghost.new_position = ball_model->position;
    latest_ghost.new_velocity = ball->velocity * time_step;

    v2 original_ghost_velocity = latest_ghost.new_velocity;
    const float ghost_radius = ball_model->scale.x / 2.0f;

    // Resolve until at destination
    while(true)
    {
      // No collision ghost
      {
        Ghost no_collision_ghost;

        // Move the rest of the velocity
        no_collision_ghost.new_position = latest_ghost.new_position + latest_ghost.new_velocity;

        // Set up the ghost.new_velocity to the ball's original velocity
        if(length_squared(latest_ghost.new_velocity) == 0.0f) no_collision_ghost.new_velocity = v2(0.0f, 0.0f);
        else no_collision_ghost.new_velocity = unit(latest_ghost.new_velocity) * length(original_ghost_velocity);

        static const float STILL_EPSILON = 0.0001f;
        if(length(no_collision_ghost.new_velocity) <= STILL_EPSILON) no_collision_ghost.new_velocity = v2(0.0f, 0.0f);

        no_collision_ghost.travel_t = 1.0f;
        no_collision_ghost.done = true;

        add_ghost(&no_collision_ghost);
      }

      // Players
      {
        Ghost player_collision_ghost;

        // Sweep circle to check for collisions against player hits
        // TODO: Deal with multiple player hits
        Player *hit_player = nullptr;
        float travel_t = 1.0f;
        check_against_player_hits(ball, latest_ghost.new_position, latest_ghost.new_velocity, ghost_radius, &travel_t, &hit_player);
        if(hit_player != nullptr)
        {
          player_collision_ghost.new_velocity = unit(hit_player->hit_direction) * (ball->hit_speed * time_step);
          player_collision_ghost.new_position = latest_ghost.new_position + latest_ghost.new_velocity * travel_t;
          player_collision_ghost.travel_t = travel_t;
          player_collision_ghost.done = true;

          player_collision_ghost.hit_player = hit_player; // For adding this ball to it's list

          add_ghost(&player_collision_ghost);
        }
      }


      // Walls
      {
        Ghost wall_collision_ghost;

        // Sweep circle to check for collision against obstacles
        float dist_t;
        v2 normal;
        bool hit_wall = check_against_walls(latest_ghost.new_position, latest_ghost.new_velocity, ghost_radius, &dist_t, &normal);
        if(hit_wall)
        {
          // Calculate new velocity
          v2 vel_to_object = latest_ghost.new_velocity * dist_t;
          vel_to_object -= unit(vel_to_object) * PAD_EPSILON;
          v2 to_object_pos = latest_ghost.new_position + vel_to_object;
          v2 vel_after_object = latest_ghost.new_velocity * (1.0f - dist_t);
          vel_after_object = reflect(vel_after_object, normal);

          // Update ghost
          wall_collision_ghost.new_velocity = vel_after_object;
          wall_collision_ghost.new_position = to_object_pos;
          wall_collision_ghost.travel_t = dist_t;

          add_ghost(&wall_collision_ghost);
        }
      }

      // Goals
      {
        Ghost goal_collision_ghost;

        float dist_t;
        v2 normal;
        bool hit_goal = check_against_goals(latest_ghost.new_position, latest_ghost.new_velocity, ghost_radius, &dist_t, &normal);
        if(hit_goal)
        {
          goal_collision_ghost.new_velocity = v2();
          goal_collision_ghost.new_position = latest_ghost.new_velocity * dist_t;
          goal_collision_ghost.travel_t = dist_t;
          goal_collision_ghost.done = true;
          goal_collision_ghost.goal = true;

          add_ghost(&goal_collision_ghost);
        }
      }

      // Pick the closest ghost
      Ghost closest_ghost;
      closest_ghost.travel_t = INFINITY;
      for(int i = 0; i < num_ghost_collisions; i++)
      {
        if(ghost_collisions[i].travel_t < closest_ghost.travel_t)
        {
          closest_ghost = ghost_collisions[i];
        }
      }

      // Debug draw
      if(debug_draw_ball_collisions)
      {
        debug_draw_line(latest_ghost.new_position, latest_ghost.new_position + latest_ghost.new_velocity * closest_ghost.travel_t);
        debug_draw_circle(latest_ghost.new_position, ghost_radius);
      }

      // Update the latest ghost
      latest_ghost = closest_ghost;

      num_ghost_collisions = 0;

      if(latest_ghost.done) break;
    }



    // TODO (2/8/2020): Too braindead to clean up this logic rn...
    // If debug draw on, only move ball if space is pressed, otherwise always move ball

    // Finalize the ball
    if(debug_draw_ball_collisions)
    {
      if(key_toggled_down('1'))
      {
        // If the ghost hit a player, add the ball to the player's last frame hit list
        if(latest_ghost.hit_player)
        {
          Player *hit_player = latest_ghost.hit_player;
          assert(hit_player->num_balls_hit_last_frame < Player::MAX_BALLS_HIT);
          hit_player->balls_hit_last_frame[hit_player->num_balls_hit_last_frame++] = ball->parent;

          ball->hit_speed += ball->hit_speed_increment;
        }

        ball_model->position = latest_ghost.new_position;
        ball->velocity = latest_ghost.new_velocity * (1.0f / time_step);
        v2 drag_vector = -ball->velocity * ball->drag * time_step;
        if(dot(ball->velocity, ball->velocity + drag_vector) > 0.0f) ball->velocity += drag_vector;

        if(latest_ghost.goal)
        {
          ball->velocity = v2();
          ball_model->position = v2();
        }
      }
    }
    else
    {
      // If the ghost hit a player, add the ball to the player's last frame hit list
      if(latest_ghost.hit_player)
      {
        Player *hit_player = latest_ghost.hit_player;
        assert(hit_player->num_balls_hit_last_frame < Player::MAX_BALLS_HIT);
        hit_player->balls_hit_last_frame[hit_player->num_balls_hit_last_frame++] = ball->parent;

        ball->hit_speed += ball->hit_speed_increment;
      }

      ball_model->position = latest_ghost.new_position;
      ball->velocity = latest_ghost.new_velocity * (1.0f / time_step);
      v2 drag_vector = -ball->velocity * ball->drag * time_step;
      if(dot(ball->velocity, ball->velocity + drag_vector) > 0.0f) ball->velocity += drag_vector;

      if(latest_ghost.goal)
      {
        ball->velocity = v2();
        ball_model->position = v2();
      }
    }

    if(ImGui::Button("Blast")) ball->velocity += unit(v2(-1.0f, 1.0f)) * 100.0f;
    ++ball_it;
  }

  ImGui::Checkbox("ball collision geometry" , &debug_draw_ball_collisions);






  // Debug draw to test line line collision etc.

#if 0
  Line a = Line(v2(), v2(2.0f, 2.0f));
  Line b = Line(v2(0.0f, 2.0f), v2(2.0f, 0.0f));
  static Circle circle = Circle(v2(4.0f, 3.0f), 1.0f);

  if(button_state(0, INPUT_SWING)) circle.position = mouse_world_position();


  a.b = mouse_world_position();

  Color c = Color(1.0f, 0.0f, 0.0f);
  float t = 0.0f;
  v2 n;

#if 0
  if(line_line_collision(a, b, &t, &n))
  {
    c = Color(1.0f, 1.0f, 0.0f);
    debug_draw_circle(a.a + (a.b - a.a) * t, 0.1f);
    debug_draw_line(b.a + (b.b - b.a) * 0.5f, b.a + (b.b - b.a) * 0.5f + n, Color(0.5f, 0.5f, 0.0f));
    ImGui::DragFloat("t", &t);
  }
#endif

  if(line_circle_collision(a, circle, &t, &n))
  {
    c = Color(1.0f, 1.0f, 0.0f);
    debug_draw_circle(a.a + (a.b - a.a) * t, 0.1f);
  }

  debug_draw_line(a.a, a.b, c);
  debug_draw_line(b.a, b.b, c);
  debug_draw_circle(circle.position, circle.radius, c);
#endif
}



