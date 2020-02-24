
#include "../networking_system.h"

#include "../game_state_system.h"
#include "../game_input.h"

#include <stdio.h>

#ifdef WIN32

#include <WinSock2.h> // Networking API
#include <Ws2tcpip.h> // InetPton

#pragma comment (lib, "Ws2_32.lib")

#define CODE_WOULD_BLOCK WSAEWOULDBLOCK
#define CODE_IS_CONNECTED WSAEISCONN
#define CODE_INVALID WSAEINVAL

#else

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <unistd.h>

typedef int SOCKET;

#define CODE_WOULD_BLOCK EAGAIN
#define CODE_IS_CONNECTED EISCONN
#define CODE_INVALID ECONNREFUSED

#endif // WIN32





static int get_last_error()
{
#ifdef WIN32
  return WSAGetLastError();
#else
  return errno;
#endif
}





class StreamConnection
{
public:
  void create_socket()
  {
    tcp_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(tcp_socket == -1) fprintf(stderr, "Error opening socket: %i\n", get_last_error());
  }

  void assign_socket(SOCKET in_socket, sockaddr_in *in_address)
  {
    tcp_socket = in_socket;
    other_address = *in_address;
  }

  void make_nonblocking()
  {
    unsigned long mode = 1;
#ifdef WIN32
    int error = ioctlsocket(tcp_socket, FIONBIO, &mode);
#else
    int error = ioctl(tcp_socket, FIONBIO, &mode);
#endif
    if(error != 0)  fprintf(stderr, "Error opening socket: %i\n", get_last_error());
  }

  int connect_to_host(const char *address_string, int port)
  {
    // Create address
    sockaddr_in address = {};
    address.sin_family = AF_INET;
    address.sin_port = htons((unsigned short)port);
    int error = inet_pton(AF_INET, address_string, &address.sin_addr.s_addr);
    if(error == -1) fprintf(stderr, "Error creating an address: %i\n", error);

    // Try to connect
    int code = connect(tcp_socket, (sockaddr *)(&address), sizeof(address));
    int status = get_last_error();

    // While not connected, try to reconnect
    while(code != 0 && status != CODE_IS_CONNECTED)
    {
      code = connect(tcp_socket, (sockaddr *)(&address), sizeof(address));
      status = get_last_error();

      if(status == CODE_INVALID) { error = CODE_INVALID; break; }
    }

    other_address = address;

    return error;
  }

  int send_data(const void *data, unsigned bytes)
  {
    // Send data over TCP
    long int bytes_queued = send(tcp_socket, (const char *)data, bytes, 0);
    if(bytes_queued == -1) fprintf(stderr, "Error sending data: %i\n", get_last_error());

    return static_cast<int>(bytes_queued); // Can safely cast because of passing in bytes as max bytes
  }

  int bytes_available()
  {
    u_long result;
    ioctlsocket(tcp_socket, FIONREAD, &result);
    return result;
  }

  bool recieve_data(char *buffer, int bytes, int *bytes_read)
  {
    // Listen for a response
    long int bytes_read_from_socket = recv(tcp_socket, buffer, bytes, 0);

    *bytes_read = static_cast<int>(bytes_read_from_socket); // Can safely cast because of passing in bytes as max bytes

    // Check if bytes were read
    if(*bytes_read == -1)
    {
      // Bytes weren't read, make sure there's an expected error code
      if(get_last_error() != CODE_WOULD_BLOCK)
      {
        fprintf(stderr, "Error recieving data: %i\n", get_last_error());
      }
      return false;
    }
    else
    {
      // Bytes were read
      return true;
    }
  }

  void no_more_sending()
  {
#ifdef WIN32
    int error = shutdown(tcp_socket, SD_SEND);
#else
    int error = shutdown(tcp_socket, SHUT_WR);
#endif
    if(error != 0) fprintf(stderr, "Error shutting down: %i\n", get_last_error());
  }

  void close_socket()
  {
    // Close the socket
#ifdef WIN32
    int error = closesocket(tcp_socket);
#else
    int error = close(tcp_socket);
#endif
    if(error == -1) fprintf(stderr, "Error closing socket: %i\n", get_last_error());
  }



private:
  SOCKET tcp_socket;
  sockaddr_in other_address;
};


struct NetworkData
{
  // Client data
  StreamConnection server_connection; 
  int client_recieve_buffer_size;
  void *client_recieve_buffer;

  // Server data
  SOCKET listening_socket;

  static const int MAX_CLIENTS = 4;
  int num_client_connections = 0;
  StreamConnection client_connections[MAX_CLIENTS];
};

static NetworkData *network_data;
static const int READ_CHUNK_SIZE = 4096;






static void init_winsock()
{
#ifdef WIN32
  int error;

  // Initialize Winsock
  WSADATA wsa_data;
  error = WSAStartup(MAKEWORD(2, 2), &wsa_data);
  if(error) fprintf(stderr, "Error loading networking module: %i\n", get_last_error());
#endif
}

static void cleanup_winsock()
{
#ifdef WIN32
  // Shut down Winsock
  int error = WSACleanup();
  if(error == -1) fprintf(stderr, "Error unloading networking module: %i\n", get_last_error());
#endif
}


static void make_listening_socket()
{
  // Create a listening socket
  SOCKET listening_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if(listening_socket == INVALID_SOCKET)
  {
    fprintf(stderr, "Couldn't create listening socket: %d\n", get_last_error());
  }

  unsigned long mode = 1;
  int error = ioctlsocket(listening_socket, FIONBIO, &mode);
  if(error != 0)  fprintf(stderr, "Error opening socket: %i\n", get_last_error());

  sockaddr_in bound_address;
  bound_address.sin_family = AF_INET;
  bound_address.sin_addr.s_addr = inet_addr("127.0.0.1");
  bound_address.sin_port = htons(4242);

  error = bind(listening_socket, (SOCKADDR *)(&bound_address), sizeof(bound_address));
  if(error == SOCKET_ERROR)
  {
    error = get_last_error();
    fprintf(stderr, "Couldn't bind listening socket: %d\n", error);
  }

  error = listen(listening_socket, SOMAXCONN);
  if(error == SOCKET_ERROR)
  {
    fprintf(stderr, "Couldn't listen on the created socket: %d\n", get_last_error());
  }

  network_data->listening_socket = listening_socket;
}






















////////////////////////////////////////////////////////////////////////////////
// Client
////////////////////////////////////////////////////////////////////////////////
void connect_to_server(const char *ip_string, int port)
{
  network_data->server_connection.connect_to_host(ip_string, port);
}

void send_input_to_server()
{
  TickInput current_tick_input = get_current_input(0);

  network_data->server_connection.send_data(&current_tick_input, sizeof(TickInput));
}

void recieve_game_state_from_server()
{
  int recieved_bytes_so_far = 0;
  bool recieved_new_data = false;

  // Read in entire stream
  do
  {
    // Recieve the data
    int recieved_bytes;
    char *buffer_location = (char *)network_data->client_recieve_buffer + recieved_bytes_so_far;
    recieved_new_data = network_data->server_connection.recieve_data(buffer_location, READ_CHUNK_SIZE, &recieved_bytes);
    recieved_bytes_so_far += recieved_bytes;

    // Check if the buffer should resize to fit another chunk
    if(recieved_bytes_so_far > network_data->client_recieve_buffer_size - READ_CHUNK_SIZE)
    {
      // Resize buffer to next read
      void *new_buffer = malloc(network_data->client_recieve_buffer_size + READ_CHUNK_SIZE);
      memcpy(new_buffer, network_data->client_recieve_buffer, network_data->client_recieve_buffer_size);
      free(network_data->client_recieve_buffer);
      network_data->client_recieve_buffer = new_buffer;
      network_data->client_recieve_buffer_size += READ_CHUNK_SIZE;
    }

  } while(recieved_new_data);

  if(recieved_bytes_so_far == SOCKET_ERROR) return;

  // Deserialze game state
  deserialize_game_state(network_data->client_recieve_buffer, recieved_bytes_so_far);
}





////////////////////////////////////////////////////////////////////////////////
// Server
////////////////////////////////////////////////////////////////////////////////
void distribute_game_state()
{
  // Serialize game state into a stream
  int bytes;
  void *stream;
  serialize_game_state(&stream, &bytes);
  
  // Send to client
  for(int i = 0; i < network_data->num_client_connections; i++)
  {
    StreamConnection *client = &(network_data->client_connections[i]);
    client->send_data(stream, bytes);
  }


  free(stream);
}

void accept_client_connections()
{
  int error = 0;
  do
  {
    if(network_data->num_client_connections >= NetworkData::MAX_CLIENTS) return;

    sockaddr_in client_address;
    int client_address_size = sizeof(sockaddr_in);
    SOCKET incoming_socket = accept(network_data->listening_socket, (sockaddr *)(&client_address), &client_address_size);
    if(incoming_socket == INVALID_SOCKET)
    {
      error = get_last_error();
      if(error != CODE_WOULD_BLOCK) fprintf(stderr, "Error on accepting socket: %d\n", get_last_error());
      continue;
    }

    // Valid socket
    StreamConnection *new_connection = &(network_data->client_connections[network_data->num_client_connections]);
    network_data->num_client_connections++;
    new_connection->assign_socket(incoming_socket, &client_address);

  } while(error != CODE_WOULD_BLOCK);
}








void init_network_system()
{
  init_winsock();

  network_data = (NetworkData *)malloc(sizeof(NetworkData));

  // Client
  network_data->client_recieve_buffer_size = READ_CHUNK_SIZE;
  network_data->client_recieve_buffer = malloc(READ_CHUNK_SIZE);
  network_data->server_connection.create_socket();
  network_data->server_connection.make_nonblocking();

  // Server
  network_data->num_client_connections = 0;
  make_listening_socket();
}

void shutdown_network_client()
{
  cleanup_winsock();

  free(network_data);
}

