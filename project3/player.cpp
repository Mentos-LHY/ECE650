#include <sys/time.h>
#include <sys/types.h>

#include <algorithm>
#include <cassert>

#include "tcp_actions.h"

using namespace std;

int main(int argc, char * argv[]) {
  if (argc != 3) {
    cerr << "Wrong argument format: player <machine_name> <port_num>" << endl;
    return EXIT_FAILURE;
  }

  const char * hostname = argv[1];
  const char * port = argv[2];

  //as client connect to ringmaster
  int ring_socket_fd = clientSetup(port, hostname);

  int num_p, p_id;
  //
  recv(ring_socket_fd, &p_id, sizeof(p_id), MSG_WAITALL);
  recv(ring_socket_fd, &num_p, sizeof(num_p), MSG_WAITALL);
  cout << "Connected as player " << p_id << " out of " << num_p << " total players\n";

  //as server itself
  int self_fd = serverSetup("");
  int self_port = getPort(self_fd);
  send(ring_socket_fd, &self_port, sizeof(self_port), 0);

  //receive neighbor player's info from ringmaster
  char p_next_ip[256];
  int p_next_port;
  recv(ring_socket_fd, &p_next_ip, sizeof(p_next_ip), MSG_WAITALL);
  recv(ring_socket_fd, &p_next_port, sizeof(p_next_port), MSG_WAITALL);

  //as client connect to neighbor server
  char p_next_port_char[10];
  sprintf(p_next_port_char, "%d", p_next_port);
  int p_next_fd = clientSetup(p_next_port_char, p_next_ip);
  int p_next_id = (p_id + 1) % num_p;

  //as server accept neighbor connection
  string p_former_ip;
  int p_former_fd = serverAccept(self_fd, &p_former_ip);
  int p_former_id = (p_id - 1 + num_p) % num_p;

  //
  //start game
  srand((unsigned int)time(NULL) + p_id);
  vector<int> three_fds{p_next_fd, p_former_fd, ring_socket_fd};
  int neighbor_id[2] = {p_next_id, p_former_id};
  Potato potato;

  while (1) {
    fd_set readfds;
    FD_ZERO(&readfds);
    vector<int>::iterator i;
    for (i = three_fds.begin(); i != three_fds.end(); ++i) {
      FD_SET(*i, &readfds);
    }
    int nfds = *max_element(three_fds.begin(), three_fds.end());
    if (select(nfds + 1, &readfds, NULL, NULL, NULL) == -1) {
      cerr << "Fails to select in player.cpp" << endl;
    }
    //
    int rv[3] = {1, 1, 1};
    for (size_t i = 0; i < three_fds.size(); i++) {
      if (FD_ISSET(three_fds[i], &readfds)) {
        rv[i] = recv(three_fds[i], &potato, sizeof(potato), MSG_WAITALL);
        break;
      }
    }
    // receive shut down signal
    if (potato.game_to_start == 0 || potato.check_finish() == true ||
        rv[0] * rv[1] * rv[2] == 0) {
      break;
    }
    //send back potato to ringmaster
    else if (potato.passed_hops == potato.total_hops - 1) {
      potato.trace[potato.passed_hops] = p_id;
      potato.passed_hops++;
      send(ring_socket_fd, &potato, sizeof(potato), 0);
      cout << "I'm it" << endl;
    }
    //else send potato to neighbor randomly
    else {
      potato.trace[potato.passed_hops] = p_id;
      potato.passed_hops++;
      int random = rand() % 2;  // 0 or 1
      send(three_fds[random], &potato, sizeof(potato), 0);
      cout << "Sending potato to " << neighbor_id[random] << endl;
    }
  }

  //shut down game
  for (size_t i = 0; i < three_fds.size(); i++) {
    close(three_fds[i]);
  }
  return EXIT_SUCCESS;
}
