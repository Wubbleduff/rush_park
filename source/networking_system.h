
#pragma once

void connect_to_server(const char* ip_string, int port);
void send_input_to_server();
void recieve_game_state_from_server();

void read_input_from_clients();
void accept_client_connections();
void distribute_game_state(unsigned tick_number);


void init_server();
void init_network_system();
