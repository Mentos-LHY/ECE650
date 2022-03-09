#pragma once

#include <stdio.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>

class Potato {
 public:
  int total_hops;
  int passed_hops;
  int trace[512];
  int game_to_start;
  Potato() : total_hops(0), passed_hops(0), game_to_start(1) {
    memset(&trace, 0, sizeof(trace));
  }

  void setHops(int & num_h) {
    this->total_hops = num_h;
    if (total_hops == 0) {
      game_to_start = 0;
    }
  }

  bool check_finish() {
    if (passed_hops == total_hops) {
      return true;
    }
    else {
      return false;
    }
  }

  void printTrace() {
    std::cout << "Trace of potato:" << std::endl;
    for (int i = 0; i < total_hops - 1; i++) {
      printf("%d,", trace[i]);
    }
    printf("%d\n", trace[total_hops - 1]);
  }
};
