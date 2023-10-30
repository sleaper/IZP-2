#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef NDEBUG
#define pmesg(s, ...) fprintf(stderr, s, __VA_ARGS__)
#else
#define pmesg(s, ...) (0)
#endif

#define MAX_LINE_LENGTH 100
#define CELL_NEIGHBORS_COUNT 3

typedef struct {
  int rows;
  int cols;
  unsigned char *cells;
} Map;

bool isborder(Map *map, int r, int c, int border);
bool valid_borders(unsigned char cell, unsigned char *neighbors);
void print_help();
int start_border(Map *map, int r, int c, int leftright);
bool maze_test(char *file_name);

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

  pmesg("Hello world %d\n", 0);
  return 0;
}

/* Returns false for INVALID maze file
 */
bool maze_test(char *file_name) {
  FILE *fptr;

  fptr = fopen(file_name, "r");
  if (fptr == NULL) {
    fprintf(stderr, "Error opening file!\n");
    return false;
  }

  int size[2] = {0};

  // Load size
  for (int i = 0; i < 2; i++) {
    fscanf(fptr, "%d", &size[i]);
  }

  if (size[0] == 0 || size[1] == 0) {
    fprintf(stderr, "Size is 0x0?\n");
    return false;
  }

  // Test size & load maze
  unsigned char cell_grid[size[0] * size[1]];
  Map maze = {.rows = size[0], .cols = size[1], .cells = cell_grid};

  int count = 0;
  int tmp = 0;
  while (fscanf(fptr, "%d", &tmp) != EOF) {
    if (tmp < 0 || tmp > 255) {
      fprintf(stderr, "Number for cell is out of range!\n");
    }

    if (count >= (maze.rows * maze.cols)) {
      fprintf(stderr, "Invalid size! %d\n", maze.rows * maze.cols);
      return false;
    }

    // Save value
    maze.cells[count] = (unsigned char)tmp;

    count++;
  }

  pmesg("rows: %d, columns: %d\n", maze.rows, maze.cols);
  pmesg("count: %d\n", count);

  //  get neigbourts around one cell
  for (int i = 0; i < maze.rows; i++) {
    for (int j = 0; j < maze.cols; j++) {
      unsigned char neighbors[3];

      neighbors[0] = (j > 0) ? maze.cells[(i * maze.cols) + j - 1] : 10;
      neighbors[1] =
          (j < maze.cols - 1) ? maze.cells[(i * maze.cols) + j + 1] : 10;

      if ((((i * maze.cols) + j) % 2) == 0) {
        neighbors[2] = (i > 0) ? maze.cells[(i - 1) * maze.cols + j] : 10;
      } else {
        neighbors[2] =
            (i < maze.rows - 1) ? maze.cells[(i + 1) * maze.cols + j] : 10;
      }

      valid_borders(maze.cells[i * maze.cols + j], neighbors);

      // pmesg("%d ",
      //       maze.cells[i * maze.cols +
      //                  j]); // current row * max column + current column
    }
    printf("\n");
  }

  fclose(fptr);
  return true;
}

bool valid_borders(unsigned char cell, unsigned char *neighbors) {
  for (int i = 0; i < 3; i++) {
    printf("neighbor: %d\n", neighbors[i]);
  }

  // Left neighbor
  if (neighbors[0] != 10 && (((cell >> 0) & 1) != ((neighbors[0] >> 1) & 1))) {
    fprintf(stderr, "Invalid left border\n");
    return false;
  }
  // Right neighbor
  if (neighbors[1] != 10 && (((cell >> 1) & 1) != ((neighbors[1] >> 0) & 1))) {
    fprintf(stderr, "Invalid right border\n");
    return false;
  }
  // Up/Down neighbor
  if (neighbors[2] != 10 && (((cell >> 2) & 1) != ((neighbors[2] >> 2) & 1))) {
    fprintf(stderr, "Invalid up/down border\n");
    return false;
  }
  printf("\n");

  return true;
}

// 0*2^0 + 1*2^1 + 0*2^2 = 2
// 0*2^0 + 0*2^1 + 1*2^2 = 4
// 1*2^0 + 0*2^1 + 1*2^2 = 5
// leva  + prava + spodni
// 0			 1  		 2
bool isborder(Map *map, int r, int c, int border) {
  return (map->cells[r * map->cols + c] >> border) & 1 ? true : false;
}

void print_help() {
  char *help = "maze [command] <options>\n\
\n\
--test <file> check, if given file has needed maze definition. If the maze file is valid program pritns VALID. Otherwise prints INVALID\n\
--rpath <R> <C> tries to find path in maze on entry on column R and row C. Path is searched by rule of right hand\n\
--lpath <R> <C> trues to find path in maze on entry on column R and row C. Path is searched by the rule of left hand";

  printf("%s", help);
}
