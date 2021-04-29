#include <stdio.h>
#include <stdlib.h>

void dieWithError(int error) {
  printf("Error: %d, exiting\n",error);
  exit(2);
}
