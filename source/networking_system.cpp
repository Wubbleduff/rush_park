
#include "networking_system.h"

#include "game_state_system.h"

static void send_stream_to_client()
{
}

void distribute_game_state()
{
  // Serialize game state into a stream
  int bytes;
  void *stream;
  serialize_game_state(&stream, &bytes);
  
  // Send to client
  send_stream_to_client();
}
