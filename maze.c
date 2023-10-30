#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define MAX_LINE_LENGTH 100

typedef struct {
  int rows;
  int cols;
  unsigned char *cells;
} Map;

bool isborder(Map *map, int r, int c, int border);
void print_help();
int start_border(Map *map, int r, int c, int leftright);
void maze_test(char *file_name);

// TODO: remove later
void showbits(unsigned char x) {
  int i = 0;
  for (i = (sizeof(char) * 8) - 1; i >= 0; i--) {
    putchar(x & (1u << i) ? '1' : '0');
  }
  printf("\n");
}

int main(int argc, char **argv) {
  // TODO: Maybe move to function?
  if (argc == 2 && !strcmp(argv[1], "--help")) {
    print_help();
    return 0;
  } else if (argc == 3 && !strcmp(argv[1], "--test")) {
    fprintf(stderr, "Running maze test\n");
    maze_test(argv[2]);
    return 0;
  }

  // for (int i = 0; argv[1][i] != '\0'; i++) {
  //   printf("%c", argv[1][i]);
  // }
  // printf("\n");

  printf("Hello world\n");
  return 0;
}

void maze_test(char *file_name) {
  FILE *fptr;
  char line[MAX_LINE_LENGTH];

  fptr = fopen(file_name, "r");
  if (fptr == NULL) {
    fprintf(stderr, "Error opening file!");
    return;
  }

  while (fgets(line, MAX_LINE_LENGTH, fptr)) {
    printf("%s", line);
  }

  fclose(fptr);
}

// 0*2^0 + 1*2^1 + 0*2^2 = 2
// 0*2^0 + 0*2^1 + 1*2^2 = 4
// 1*2^0 + 0*2^1 + 1*2^2 = 5
// leva  + prava + spodni
// 0			 1  		 2
bool isborder(Map *map, int r, int c, int border) {
  return (map->cells[r * c] >> border) & 1 ? true : false;
}

void print_help() {
  char *help = "maze [command] <options>\n\
\n\
--test <file> check, if given file has needed maze definition. If the maze file is valid program pritns VALID. Otherwise prints INVALID\n\
--rpath <R> <C> tries to find path in maze on entry on column R and row C. Path is searched by rule of right hand\n\
--lpath <R> <C> trues to find path in maze on entry on column R and row C. Path is searched by the rule of left hand";

  printf("%s", help);
}
