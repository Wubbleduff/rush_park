
#include "networking_system.h"

#include "game_state_system.h"

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

class StreamConnection
{
public:
  void create_socket()
  {
    tcp_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(tcp_socket == -1) fprintf(stderr, "Error opening socket: %i\n", get_last_error());
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

    return error;
  }

  int send_data(const void *data, unsigned bytes)
  {
    // Send data over TCP
    long int bytes_queued = send(tcp_socket, (const char *)data, bytes, 0);
    if(bytes_queued == -1) fprintf(stderr, "Error sending data: %i\n", get_last_error());

    return static_cast<int>(bytes_queued); // Can safely cast because of passing in bytes as max bytes
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

  void close_connection()
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
};


struct NetworkData
{
  StreamConnection client_connection;
};

static NetworkData *network_data;































void distribute_game_state()
{
  // Serialize game state into a stream
  int bytes;
  void *stream;
  serialize_game_state(&stream, &bytes);
  
  // Send to client
  //send_stream_to_client(stream, bytes);


  free(stream);
}


void init_network_server(int port)
{
  init_winsock();

  network_data = (NetworkData *)malloc(sizeof(NetworkData));

  StreamConnection *connection = &(network_data->client_connection);
}

void shutdown_network_client()
{
  cleanup_winsock();

  free(network_data);
}

