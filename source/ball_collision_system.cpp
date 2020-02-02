
#include "ball_collision_system.h"
#include "component.h"

#include <assert.h>


const static int MAX_BALL_COLLISIONS = 128;

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
  const ColliderComponent *a;
  const ColliderComponent *b;

  float dist_t;
  v2 normal;
};

#if 0
static bool circle_bb_collision(const ColliderComponent *a_collider, const ColliderComponent *b_collider, BallCollisionInfo *info)
{
  assert(a_collider->type == ColliderComponent::CIRCLE);
  assert(b_collider->type == ColliderComponent::RECT);

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
      v2 x_point = line.a + d * t1;
      *normal = x_point - circle.position;
      return true ;
    }

    // here t1 didn't intersect so we are either started
    // inside the sphere or completely past it
    if(t2 >= 0.0f && t2 <= 1.0f)
    {
      // ExitWound
      return false;
    }
    // no intersection: FallShort, Past, CompletelyInside
    return false ;
  }
}

// dist_t returns the distance percentage along a
// normal returns the normal to b
static bool line_line_collision(Line a, Line b, float *dist_t, v2 *normal)
{
  return false;
}

static void check_swept_ball(ColliderComponent *ball_collider, v2 ball_position, v2 ball_velocity, float ball_radius, BallCollisionInfo *collisions, int *num_collisions)
{
  *num_collisions = 0;

  // Go through all other colliders
  ComponentIterator<ColliderComponent> collider_it = get_colliders_iterator();
  while(collider_it != nullptr)
  {
    // Get the other collider
    ColliderComponent *other_collider = &(*collider_it);
    ModelComponent *other_model = (ModelComponent *)other_collider->parent.get_component(C_MODEL);
    if(ball_collider == other_collider) { ++collider_it; continue; }
    
    // Create the ball's line
    Line travel_line = Line(ball_position, ball_position + ball_velocity);




    // Sweep circle against other rectangle
    assert(other_collider->type == ColliderComponent::RECT);

    // Create the corners and edges
    Circle corners[4];
    Line edges[4];
    {
      v2 rect_pos = other_model->position;
      v2 half_scale = other_collider->rect.scale / 2.0f;

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
    if(hit_anything)
    {
      BallCollisionInfo info;
      info.a = ball_collider;
      info.b = other_collider;
      info.dist_t = closest_dist_t;
      info.normal = closest_normal;

      assert(*num_collisions < MAX_BALL_COLLISIONS);
      collisions[*num_collisions] = info;
      *num_collisions++;
    }

    ++collider_it;
  }
}





#if 0
void update_ball_collision_system(float time_step)
{
  ComponentIterator<BallComponent> ball_it = get_balls_iterator();

  // Check collisions
  while(ball_it != nullptr)
  {
    BallComponent *ball_component = &(*ball_it);
    ColliderComponent *ball_collider = (ColliderComponent *)ball_component->parent.get_component(C_COLLIDER);

    assert(ball_collider->type == ColliderComponent::CIRCLE);

    ComponentIterator<ColliderComponent> collider_it = get_colliders_iterator();
    while(collider_it != nullptr)
    {
      ColliderComponent *other_collider = &(*collider_it);

      if(ball_collider == other_collider) { ++collider_it; continue; }
      
      assert(other_collider->type == ColliderComponent::RECT);

      BallCollisionInfo info;
      ModelComponent *ball_model = (ModelComponent *)ball_component->parent.get_component(C_MODEL);
      PlayerComponent *player = (PlayerComponent *)other_collider->parent.get_component(C_PLAYER);
      if(circle_bb_collision(ball_collider, other_collider, &info))
      {
        if(player == nullptr)
        {
          ball_component->velocity = reflect(ball_component->velocity, info.normal);
        }
      }



      ++collider_it;
    }

    ++ball_it;
  }




  // Resolve collisions




  ball_it = get_balls_iterator();
  while(ball_it != nullptr)
  {
    BallComponent *ball = &(*ball_it);
    ModelComponent *model = (ModelComponent *)ball->parent.get_component(C_MODEL);


    ball->velocity -= ball->velocity * ball->drag * time_step;
    model->position += ball->velocity * ball->speed * time_step;


    ++ball_it;
  }
}
#endif



#if 1

void update_ball_collision_system(float time_step)
{
  return;

  // For each ball
  //   while will not collide
  //     sweep circle along travel path and get collision info
  //     if no collisions: break
  //     pick "earliest" collision
  //     decide how to resolve the collision based on what it collided with
  //     resolve
  //

  ComponentIterator<BallComponent> ball_it = get_balls_iterator();

  static int num_collisions = 0;
  static BallCollisionInfo collisions[MAX_BALL_COLLISIONS];

  // For each ball
  while(ball_it != nullptr)
  {
    BallComponent *ball = &(*ball_it);
    ModelComponent *ball_model = (ModelComponent *)ball->parent.get_component(C_MODEL);
    ColliderComponent *ball_collider = (ColliderComponent *)ball->parent.get_component(C_COLLIDER);
    assert(ball_collider->type == ColliderComponent::CIRCLE);

    v2 ghost_position = ball_model->position;
    v2 ghost_velocity = ball->velocity * time_step;
    const float ghost_radius = ball_collider->circle.radius;

    // Resolve until at destination
    while(true)
    {
      // Sweep circle along travel path and get collision info
      check_swept_ball(ball_collider, ghost_position, ghost_velocity, ghost_radius, collisions, &num_collisions);

      // Done if no more collisions will happen
      if(num_collisions == 0) break;

      // Find the earliest collision
      BallCollisionInfo *earliest_collision = nullptr;
      for(int i = 0; i < num_collisions; i++)
      {
        if(collisions[i].dist_t < earliest_collision->dist_t) earliest_collision = &(collisions[i]);
      }

      // Decide how to resolve the collision based on what it collided with
      


      // Resolve
      ghost_position += ghost_velocity * earliest_collision->dist_t;
      ghost_velocity *= earliest_collision->dist_t;
      ghost_velocity = reflect(-ghost_velocity, earliest_collision->normal);
    }


    // Move the rest of the ghost velocity
    ghost_position += ghost_velocity;


    // Finalize the ball
    ball_model->position = ghost_position;
    ball->velocity = unit(ghost_velocity) * length(ball->velocity);
    ball->velocity -= ball->velocity * ball->drag * time_step;
  }

}

#endif





