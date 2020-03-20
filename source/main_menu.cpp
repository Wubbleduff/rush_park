
#include "main_menu.h"

#include "rendering_system.h"
#include "game_input.h"
#include "engine.h"

void update_main_menu(float time_step)
{
  v2 mouse_pos = mouse_ndc_position();


  draw_rect(v2(0.0f, 0.0f), v2(2.0f, 2.0f), 0.0f, "TitleScreen");

  if(mouse_pos.x >  0.6f - 0.2f &&
     mouse_pos.x <  0.6f + 0.2f &&
     mouse_pos.y > -0.2f - 0.15f &&
     mouse_pos.y < -0.2f + 0.15f)
  {
    draw_rect(v2(0.6f, -0.2f), v2(0.4f, 0.3f), 0.2f, "StartButton");

    if(mouse_toggled_down(0))
    {
      change_engine_mode(LOCAL_GAME);
    }
  }
  else
  {
    draw_rect(v2(0.6f, -0.2f), v2(0.4f, 0.3f), 0.0f, "StartButton");
  }


  if(mouse_pos.x >  0.6f - 0.2f &&
     mouse_pos.x <  0.6f + 0.2f &&
     mouse_pos.y > -0.6f - 0.15f &&
     mouse_pos.y < -0.6f + 0.15f)
  {
    draw_rect(v2(0.6f, -0.6f), v2(0.4f, 0.3f), 0.2f, "QuitButton");

    if(mouse_toggled_down(0))
    {
      stop_engine();
    }
  }
  else
  {
    draw_rect(v2(0.6f, -0.6f), v2(0.4f, 0.3f), 0.0f, "QuitButton");
  }
}

