#include "life.hpp"

#define TEST_DIAMETER 100 // CANT HANDLE 10000, 100 is very fast
typedef life::Universe Universe;

int main(int argc, char **argv) {
  int diameter = TEST_DIAMETER;
  Universe *universe = new Universe(diameter);
  universe->startTime(); 
  return 0;
}
