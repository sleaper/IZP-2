#include <stdio.h>

// Example for 1 1
// 0*2^0 + 0*2^1 + 1*2^2 = 4

typedef struct {
  int rows;
  int cols;
  unsigned char *cells;
} Map;

int main(int argc, char **argv) {
  (void)argc;
  (void)argv;

  // Check for --help flag
  if (argc == 2) {
    printf("TODO: return HELP\n");
    return 0;
  }

  for (int i = 0; argv[2][i] != '\0'; i++) {
    printf("%c", argv[2][i]);
  }
  printf("\n");

  // Read file
  // FILE *fptr;
  //
  // // use appropriate location if you are using MacOS or Linux
  // fptr = fopen(argv[], "r");
  //
  // if (fptr == NULL) {
  //   printf("Error!");
  //   return 1;
  // }
  //
  //
  // fclose(fptr);

  printf("Hello world\n");
  return 0;
}
