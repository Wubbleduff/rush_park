
#pragma once

void set_game_to_restart();

bool should_restart_game();
bool should_simulate_game();

void reset_game_state();

void serialize_game_state(void **out_stream, int *out_bytes);
void deserialize_game_state(const void *data, int bytes);

void init_game_state_system();

