#include "tcp_actions.h"

using namespace std;

int serverSetup(const char * port) {
  struct addrinfo host_info;
  struct addrinfo * host_info_list;
  const char * hostname = NULL;
  int socket_fd;

  memset(&host_info, 0, sizeof(host_info));

  host_info.ai_family = AF_UNSPEC;
  host_info.ai_flags = AI_PASSIVE;
  host_info.ai_socktype = SOCK_STREAM;

  if (getaddrinfo(hostname, port, &host_info, &host_info_list) != 0) {
    cerr << "(server) Cannot get address info for host: "
         << "(" << hostname << "," << port << ")" << endl;
    exit(EXIT_FAILURE);
  }

  socket_fd = socket(host_info_list->ai_family,
                     host_info_list->ai_socktype,
                     host_info_list->ai_protocol);
  if (socket_fd == -1) {
    cerr << "(server) Cannot create socket: "
         << "(" << hostname << "," << port << ")" << endl;
    exit(EXIT_FAILURE);
  }

  int status;
  int yes = 1;
  status = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
  status = bind(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
  if (status == -1) {
    cerr << "(server) Cannot bind socket: "
         << "(" << hostname << "," << port << ")" << endl;
    exit(EXIT_FAILURE);
  }

  if (listen(socket_fd, 100) == -1) {
    cerr << "(server) Cannot listen on socket: "
         << "(" << hostname << "," << port << ")" << endl;
    exit(EXIT_FAILURE);
  }

  freeaddrinfo(host_info_list);
  return socket_fd;
}

int clientSetup(const char * port, const char * hostname) {
  struct addrinfo host_info;
  struct addrinfo * host_info_list;

  memset(&host_info, 0, sizeof(host_info));
  host_info.ai_family = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;

  if (getaddrinfo(hostname, port, &host_info, &host_info_list) != 0) {
    cerr << "Cannot get address info for host: "
         << "(" << hostname << "," << port << ")" << endl;
    exit(EXIT_FAILURE);
  }

  int socket_fd;
  socket_fd = socket(host_info_list->ai_family,
                     host_info_list->ai_socktype,
                     host_info_list->ai_protocol);
  if (socket_fd == -1) {
    cerr << "Cannot create socket: "
         << "(" << hostname << "," << port << ")" << endl;
    exit(EXIT_FAILURE);
  }

  if (connect(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen) == -1) {
    cerr << "Cannot connect to socket: "
         << "(" << hostname << "," << port << ")" << endl;
    exit(EXIT_FAILURE);
  }

  freeaddrinfo(host_info_list);
  return socket_fd;
}

int serverAccept(int & fd, string * ip) {
  struct sockaddr_storage socket_addr;
  socklen_t socket_addr_len = sizeof(socket_addr);
  int client_connection_fd;
  client_connection_fd = accept(fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
  if (client_connection_fd == -1) {
    cerr << "Cannot accept connection on socket from player to ringmaster" << endl;
    exit(EXIT_FAILURE);
  }
  //
  //return client's ip address in string * ip
  struct sockaddr_in * addr = (struct sockaddr_in *)&socket_addr;
  *ip = inet_ntoa(addr->sin_addr);
  //
  return client_connection_fd;
}

int getPort(int & fd) {
  struct sockaddr_in addr;
  socklen_t len = sizeof(addr);
  int status;
  status = getsockname(fd, (struct sockaddr *)&addr, &len);
  if (status == -1) {
    cerr << "Cannot get local address of the socket" << endl;
    exit(EXIT_FAILURE);
  }
  //return local tcp port number
  return ntohs(addr.sin_port);
}
