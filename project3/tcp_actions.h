#pragma once

using namespace std;

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include "potato.h"

int serverSetup(const char * port);

int clientSetup(const char * port, const char * hostname);

int serverAccept(int & fd, string * ip);

int getPort(int & fd);
