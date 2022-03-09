#include <sys/time.h>
#include <sys/types.h>

#include <cassert>

#include "tcp_actions.h"

using namespace std;

class PlayerInfo {
 public:
  int id;
  int player_fd;
  int port;
  string hostname;

  PlayerInfo() : id(0), player_fd(0), port(0), hostname("") {}

  void setInfo(int & p_id, int & p_connection_fd, int & p_port, string & p_ip) {
    id = p_id;
    player_fd = p_connection_fd;
    port = p_port;
    hostname = p_ip;
  }
};

void ring_Connect_players(int master_fd, int num, PlayerInfo * p_info) {
  for (int p_id = 0; p_id < num; p_id++) {
    string p_ip;  //ip address
    int p_port;   //player use this port as server
    int p_connection_fd = serverAccept(master_fd, &p_ip);  //accept();
    //send each player their id and game info
    send(p_connection_fd, &p_id, sizeof(p_id), 0);
    send(p_connection_fd, &num, sizeof(num), 0);

    //receive from each player their local listen port
    recv(p_connection_fd, &p_port, sizeof(p_port), MSG_WAITALL);

    p_info[p_id].setInfo(p_id, p_connection_fd, p_port, p_ip);

    cout << "Player " << p_id << " is ready to play" << endl;
  }
}

void printGameInfo(int & num_p, int & num_h) {
  cout << "Potato Ringmaster\n";
  cout << "Players = " << num_p << endl;
  cout << "Hops = " << num_h << endl;
}

//send player their next player's info to set up the ring of players
void ringSetup(int & num_p, PlayerInfo * p_info) {
  for (int p_id = 0; p_id < num_p; p_id++) {
    int p_next_id = (p_id + 1) % num_p;  //0 -> 1; N-1 -> 0
    //
    int p_next_port = p_info[p_next_id].port;
    string p_next_ip = p_info[p_next_id].hostname;
    char p_next_ip_char[256] = {0};

    //memset(p_next_ip_char, 0, sizeof(p_next_ip_char));
    strcpy(p_next_ip_char, p_next_ip.c_str());

    // ringmaster send player their next player's info
    send(p_info[p_id].player_fd, &p_next_ip_char, sizeof(p_next_ip_char), 0);
    send(p_info[p_id].player_fd, &p_next_port, sizeof(p_next_port), 0);
  }
}

void sendpotato(int & num_p, PlayerInfo * p_info, Potato & potato) {
  srand((unsigned int)time(NULL) + num_p);
  int random = rand() % num_p;

  send(p_info[random].player_fd, &potato, sizeof(potato), 0);
  //
  cout << "Ready to start the game, sending potato to player " << random << endl;
}

void shutdownGame(Potato & potato, int & num_p, PlayerInfo * p_info) {
  for (int p_id = 0; p_id < num_p; p_id++) {
    send(p_info[p_id].player_fd, &potato, sizeof(potato), 0);
  }
  if (potato.check_finish() == true) {
    potato.printTrace();
  }
  //close all player's fd
  for (int p_id = 0; p_id < num_p; p_id++) {
    close(p_info[p_id].player_fd);
  }
}

int main(int argc, char * argv[]) {
  if (argc != 4) {
    cerr << "Wrong argument format: ringmaster <port_num> <num_players> <num_hops>"
         << endl;
    return EXIT_FAILURE;
  }

  const char * ring_port = argv[1];
  int num_p = atoi(argv[2]);
  int num_h = atoi(argv[3]);

  //check players
  if (num_p <= 1) {
    cerr << "<num_players> must be larger than 1" << endl;
    return EXIT_FAILURE;
  }

  //check hops
  if (num_h > 512 || num_h < 0) {
    cerr << "<num_hops> must be in 0-512" << endl;
    return EXIT_FAILURE;
  }

  //start
  printGameInfo(num_p, num_h);

  //setup server on ringmaster
  int ring_fd = serverSetup(ring_port);

  //ringmaster accept connections from players
  //
  PlayerInfo p_info[num_p];
  //
  ring_Connect_players(ring_fd, num_p, p_info);
  ringSetup(num_p, p_info);

  //send out potato and wait for game over
  Potato potato;
  potato.setHops(num_h);

  //hops = 0
  if (potato.game_to_start == 0) {  //end the game

    /*for (int p_id = 0; p_id < num_p; p_id++) {
      send(p_info[p_id].player_fd, &potato, sizeof(potato), 0);
    }
    */
    //close and shut down game
    shutdownGame(potato, num_p, p_info);
    close(ring_fd);
    return EXIT_SUCCESS;
  }

  //hops > 0
  else {  //play the game
    //send out potato
    sendpotato(num_p, p_info, potato);

    //wait to receive potato
    while (1) {
      fd_set readfds;
      FD_ZERO(&readfds);
      int nfds = p_info[0].player_fd;
      for (int p_id = 0; p_id < num_p; p_id++) {
        FD_SET(p_info[p_id].player_fd, &readfds);  //add player_fd into readfds
        if (p_info[p_id].player_fd > nfds) {
          nfds = p_info[p_id].player_fd;
        }
      }
      //select fd
      if (select(nfds + 1, &readfds, NULL, NULL, NULL) == -1) {
        //select updates fd_set so in the following we need to FD_ISSET
        cerr << "Fails to select in ringmaster.cpp" << endl;
      }
      for (int p_id = 0; p_id < num_p; p_id++) {
        if (FD_ISSET(p_info[p_id].player_fd, &readfds)) {
          recv(p_info[p_id].player_fd, &potato, sizeof(potato), MSG_WAITALL);
          break;
        }
      }
      if (potato.check_finish() == true) {
        shutdownGame(potato, num_p, p_info);
        break;
      }
    }
  }
  close(ring_fd);
  return EXIT_SUCCESS;
}
