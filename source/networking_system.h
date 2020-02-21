
#pragma once

void connect_to_server(const char* ip_string, int port);
void send_input_to_server();

void accept_client_connections();
void distribute_game_state();


void init_network_system();
