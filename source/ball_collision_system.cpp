
#include "ball_collision_system.h"
#include "component.h"

#include "rendering_system.h" // Debug draw
#include "game_input.h"
#include "imgui.h"

#include <assert.h>


const static int MAX_BALL_COLLISIONS = 128;
const static float PAD_EPSILON = 0.05f;

static bool debug_draw_ball_collisions = false;

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

// For resolution
struct BallCollisionInfo
{
  //const EntityHandle b;

  float dist_t;
  v2 normal;
};

#if 0
static bool circle_bb_collision()
{
  ModelComponent *a_model = (ModelComponent *)a_collider->parent.get_component(C_MODEL);
  ModelComponent *b_model = (ModelComponent *)b_collider->parent.get_component(C_MODEL);

  v2 a_pos = a_model->position;
  float radius = a_collider->circle.radius;

  v2 b_pos = b_model->position;
  v2 b_half_scale = b_collider->rect.scale / 2.0f;

  float b_x_min = b_pos.x - b_half_scale.x;
  float b_x_max = b_pos.x + b_half_scale.x;
  float b_y_min = b_pos.y - b_half_scale.y;
  float b_y_max = b_pos.y + b_half_scale.y;

  v2 clamped = v2(clamp(a_pos.x, b_x_min, b_x_max), clamp(a_pos.y, b_y_min, b_y_max));

  if(length_squared(a_pos - clamped) > radius * radius) return false;

  float left_depth   = absf(b_x_min - a_pos.x);
  float right_depth  = absf(b_x_max - a_pos.x);
  float top_depth    = absf(b_y_max - a_pos.y);
  float bottom_depth = absf(b_y_min - a_pos.y);

  float depth = min(min(min(left_depth, right_depth), top_depth), bottom_depth);

  if(depth == left_depth)   info->normal = v2(-1.0f,  0.0f);
  if(depth == right_depth)  info->normal = v2( 1.0f,  0.0f);
  if(depth == top_depth)    info->normal = v2( 0.0f,  1.0f);
  if(depth == bottom_depth) info->normal = v2( 0.0f, -1.0f);

  BallComponent *ball = (BallComponent *)a_collider->parent.get_component(C_BALL);
  if(dot(ball->velocity, info->normal) > 0.0f) return false;

  return true;
}
#endif

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

static bool check_against_player_hits(v2 ball_position, v2 ball_velocity, float ball_radius, float *closest_dist_t, Player **hit_player)
{
  ComponentIterator<Player> player_it = get_players_iterator();

  *closest_dist_t = INFINITY;
  *hit_player = nullptr;

  while(player_it != nullptr)
  {
    // Make sure the player's hitting
    if(!player_it->state == Player::SWINGING) { ++player_it; continue; }

    Model *player_model = (Model *)player_it->parent.get_component(C_MODEL);

    if(debug_draw_ball_collisions) debug_draw_circle(player_model->position, ball_radius + player_it->hit_radius);

    // Check if the ball is right on top of the player
    if(circle_circle_collision(Circle(ball_position, ball_radius), Circle(player_model->position, player_it->hit_radius)))
    {
      *closest_dist_t = 0.0f;
      *hit_player = &(*player_it);
      break; // Can break since we know the ghosty ball shouldn't move to a player
    }

    // Sweep ball travel line against the player's hit
    Circle ball_player_sum_circle = Circle(player_model->position, ball_radius + player_it->hit_radius);
    Line travel_line = Line(ball_position, ball_position + ball_velocity);

    // TODO: Also check the swing arc

    float t = 0.0f;
    v2 n = v2();
    bool hit = line_circle_collision(travel_line, ball_player_sum_circle, &t, &n);
    if(hit && t < *closest_dist_t)
    {
      *closest_dist_t = t;
      *hit_player = &(*player_it);
    }

    ++player_it;
  }

  if(*hit_player == nullptr) return false;
  return true;
}

static bool check_against_walls(v2 ball_position, v2 ball_velocity, float ball_radius, BallCollisionInfo *earliest_collision)
{
  bool result = false;
  earliest_collision->dist_t = 1.0f;

  // Go through walls
  ComponentIterator<Wall> wall_it = get_walls_iterator();
  while(wall_it != nullptr)
  {
    Wall *wall = &(*wall_it);
    Model *wall_model = (Model *)wall->parent.get_component(C_MODEL);
    
    // Create the ball's line
    Line travel_line = Line(ball_position, ball_position + ball_velocity);




    // Sweep circle against other rectangle

    // Create the corners and edges
    Circle corners[4];
    Line edges[4];
    {
      v2 rect_pos = wall_model->position;
      v2 half_scale = wall_model->scale / 2.0f;

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

    // Add the final collision info
    if(hit_anything && closest_dist_t < earliest_collision->dist_t)
    {
      result = true;
      earliest_collision->dist_t = closest_dist_t;
      earliest_collision->normal = closest_normal;
    }

    ++wall_it;
  }

  return result;
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

  static int num_collisions = 0;
  static BallCollisionInfo collisions[MAX_BALL_COLLISIONS];

  // For each ball
  while(ball_it != nullptr)
  {
    Ball *ball = &(*ball_it);
    Model *ball_model = (Model *)ball->parent.get_component(C_MODEL);

    v2 ghost_position = ball_model->position;
    v2 ghost_velocity = ball->velocity * time_step;
    const float ghost_radius = ball_model->scale.x / 2.0f;

    //ghost_velocity = mouse_world_position();

    // Resolve until at destination
    while(true)
    {
      if(debug_draw_ball_collisions) debug_draw_circle(ghost_position, ghost_radius);

      // Sweep circle to check for collisions against player hits
      float to_player_hit_t = 0.0f;
      Player *hit_player = nullptr;
      bool did_hit_player = check_against_player_hits(ghost_position, ghost_velocity, ghost_radius, &to_player_hit_t, &hit_player);


      if(did_hit_player)
      {
        if(debug_draw_ball_collisions) debug_draw_line(ghost_position, ghost_position + ghost_velocity * to_player_hit_t);

        // Move the ball to the player
        ghost_position += ghost_velocity * to_player_hit_t;

        if(debug_draw_ball_collisions) debug_draw_circle(ghost_position, ghost_radius, Color(1.0f, 1.0f, 0.0f));

        // Set the ball's velocity in the direction of the hit
        ghost_velocity = hit_player->hit_direction *ball->hit_speed; 

        // Increase the ball's hit speed
        ball->hit_speed += ball->hit_speed_increment;

        // Stop doing collisions for this frame
        break;
      }



      // WALLS

      // Sweep circle to check for collision against obstacles
      BallCollisionInfo wall_collision;
      bool hit_wall = check_against_walls(ghost_position, ghost_velocity, ghost_radius, &wall_collision);

      // Done if no more collisions will happen
      if(hit_wall == false)
      {
        // Done with wall checks

        if(debug_draw_ball_collisions) debug_draw_line(ghost_position, ghost_position + ghost_velocity);

        // Move the rest of the velocity
        ghost_position += ghost_velocity;

        // Set up the ghost velocity to the ball's original velocity
        if(length_squared(ghost_velocity) == 0.0f) ghost_velocity = v2(0.0f, 0.0f);
        else ghost_velocity = unit(ghost_velocity) * length(ball->velocity);

        break;
      }

      // Calculate new velocity
      v2 vel_to_object = ghost_velocity * wall_collision.dist_t;
      vel_to_object -= unit(vel_to_object) * PAD_EPSILON;
      v2 to_object_pos = ghost_position + vel_to_object;
      v2 vel_after_object = ghost_velocity * (1.0f - wall_collision.dist_t);


      // Debug draw
      if(debug_draw_ball_collisions) debug_draw_line(ghost_position, to_object_pos);


      // Update ghost
      ghost_position += vel_to_object;
      ghost_velocity = vel_after_object;
      ghost_velocity = reflect(ghost_velocity, wall_collision.normal);
      static const float STILL_EPSILON = 0.001f;
      if(length(ghost_velocity) <= STILL_EPSILON) ghost_velocity = v2(0.0f, 0.0f);
    }



    // TODO (2/8/2020): Too braindead to clean up this logic rn...
    // If debug draw on, only move ball if space is pressed, otherwise always move ball

    // Finalize the ball
    if(debug_draw_ball_collisions)
    {
      if(button_toggled_down(0, INPUT_SWING))
      {
        ball_model->position = ghost_position;
        ball->velocity = ghost_velocity;
        ball->velocity -= ball->velocity * ball->drag * time_step;
      }
    }
    else
    {
      ball_model->position = ghost_position;
      ball->velocity = ghost_velocity;
      ball->velocity -= ball->velocity * ball->drag * time_step;
    }

    if(ImGui::Button("Blast")) ball->velocity += unit(v2(-1.0f, 1.0f)) * 30.0f;
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



