
// Platform independent
#include "../engine.cpp"
#include "../entity.cpp"
#include "../component.cpp"
#include "../game_logic_system.cpp"
#include "../player_collision_system.cpp"
#include "../player_controller_system.cpp"
#include "../ball_collision_system.cpp"

// Platform dependent
#include "renderer.cpp"
#include "main.cpp"

#include "../../libs/imgui/imgui.cpp"
#include "../../libs/imgui/imgui_demo.cpp"
#include "../../libs/imgui/imgui_draw.cpp"
#include "../../libs/imgui/imgui_impl_dx11.cpp"
#include "../../libs/imgui/imgui_impl_win32.cpp"
#include "../../libs/imgui/imgui_widgets.cpp"
