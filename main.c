#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include "ccc.h"

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "invalid number of arguments!\n");
    return 1;
  }

  tokenize(argv[1]);
  Node **code = program();

  gen_all(code);

  return 0;
}
