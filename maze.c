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

typedef struct {
  int x;
  int y;
} coordinates_t;

bool isborder(Map *map, int r, int c, int border);
bool valid_borders(unsigned char cell, unsigned char *neighbors);
void print_help();
int start_border(Map *map, int r, int c, int leftright);
int maze_test(char *file_name);
void rpath(Map *map, coordinates_t start);
void lpath(Map *map, coordinates_t start);

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
    pmesg("Running maze test %d\n", 0);
    maze_test(argv[2]);
    return 0;
  } else if (argc == 5 && !strcmp(argv[1], "--rpath")) {
    printf("RPATH");
    // rpath()
  } else {
    fprintf(stderr, "Invalid argument\n");
  }

  pmesg("Hello world argc: %d\n", argc);
  return 0;
}

int open_file(FILE *fptr, char *file_name) {
  fptr = fopen(file_name, "r");
  if (fptr == NULL) {
    fprintf(stderr, "Error opening file!\n");
    return 1;
  }
  return 0;
}

int load_maze_size(FILE *fptr, int rows, int cols) { return 0; }

/* Returns 1 as failure, 0 for success
 * TODO: Split into more functions
 */
int maze_test(char *file_name) {
  // Load file
  FILE *fptr;

  open_file(fptr, file_name);

  // Load size
  int size[2] = {0};

  for (int i = 0; i < 2; i++) {
    fscanf(fptr, "%d", &size[i]);
  }

  // Test size & load maze
  unsigned char cell_grid[size[0] * size[1]];
  Map maze = {.rows = size[0], .cols = size[1], .cells = cell_grid};

  int count = 0;
  int tmp = 0;
  while (fscanf(fptr, "%d", &tmp) != EOF) {
    if (tmp < 0 || tmp > 255) {
      fprintf(stderr, "Invalid\n");
      return 1;
    }

    if (count >= (maze.rows * maze.cols)) {
      fprintf(stderr, "Invalid\n");
      return 1;
    }

    // Save value
    maze.cells[count] = (unsigned char)tmp;

    count++;
  }

  pmesg("rows: %d, columns: %d\n", maze.rows, maze.cols);
  pmesg("count: %d\n", count);

  // get neigbourts around one cell
  for (int i = 0; i < maze.rows; i++) {
    for (int j = 0; j < maze.cols; j++) {
      unsigned char neighbors[3];

      // Left
      neighbors[0] = (j > 0) ? maze.cells[(i * maze.cols) + j - 1] : 10;
      // Right
      neighbors[1] =
          (j < maze.cols - 1) ? maze.cells[(i * maze.cols) + j + 1] : 10;

      // Top/down
      if ((((i * maze.cols) + j) % 2) == 0) {
        neighbors[2] = (i > 0) ? maze.cells[(i - 1) * maze.cols + j] : 10;
      } else {
        neighbors[2] =
            (i < maze.rows - 1) ? maze.cells[(i + 1) * maze.cols + j] : 10;
      }

      if (!valid_borders(maze.cells[i * maze.cols + j], neighbors)) {
        fprintf(stderr, "Invalid\n");
        return 1;
      }
    }
  }
  fclose(fptr);

  fprintf(stdout, "Valid\n");
  return 0;
}

bool valid_borders(unsigned char cell, unsigned char *neighbors) {
  // Left neighbor
  if (neighbors[0] != 10 && (((cell >> 0) & 1) != ((neighbors[0] >> 1) & 1))) {
    return false;
  }
  // Right neighbor
  if (neighbors[1] != 10 && (((cell >> 1) & 1) != ((neighbors[1] >> 0) & 1))) {
    return false;
  }
  // Up/Down neighbor
  if (neighbors[2] != 10 && (((cell >> 2) & 1) != ((neighbors[2] >> 2) & 1))) {
    return false;
  }

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
--test <file> check, if given file has needed maze definition. If the maze file is valid program pritns Valid. Otherwise prints Invalid\n\
--rpath <R> <C> tries to find path in maze on entry on column R and row C. Path is searched by rule of right hand\n\
--lpath <R> <C> trues to find path in maze on entry on column R and row C. Path is searched by the rule of left hand";

  printf("%s", help);
}
