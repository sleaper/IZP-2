#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef NDEBUG
#define pmesg(s, ...)                                                          \
  fprintf(stderr, "%s:%u: " s "\n", __func__, __LINE__, ##__VA_ARGS__)
#else
#define pmesg(s, ...)                                                          \
  do {                                                                         \
  } while (0)
#endif

#define MAX_LINE_LENGTH 100
#define CELL_NEIGHBORS_COUNT 3
#define EMPTY_CELL 10
#define RIGHT_RULE 6
#define LEFT_RULE 7

typedef struct {
  int rows;
  int cols;
  unsigned char *cells;
} Map;

typedef struct {
  int left;
  int right;
} coordinates_t;

bool isborder(Map *map, int r, int c, int border);
bool valid_borders(unsigned char cell, unsigned char *neighbors);
void print_help();
unsigned char *maze_test(char *file_name);
int start_border(Map *map, int r, int c, int leftright);    // TODO
unsigned char *rpath(coordinates_t start, char *file_name); // TODO
void lpath(Map *map, coordinates_t start, char *file_name); // TODO
Map *init_maze(FILE **fptr, Map *maze, char *file_name);
Map *map_load(FILE **fptr, Map *maze);

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
    pmesg("Running maze test");
    unsigned char *cells = maze_test(argv[2]);

    if (cells == NULL) {
      fprintf(stderr, "Invalid\n");
      return 1;
    } else {
      free(cells);
      fprintf(stdout, "Valid\n");
      return 0;
    }
  } else if (argc == 5 && !strcmp(argv[1], "--rpath")) {
    pmesg("RPATH");
    pmesg("START at: [%s][%s]", argv[2], argv[3]);
    coordinates_t start = {.left = atoi(argv[2]), .right = atoi(argv[3])};
    // TODO Valid start point somehow
    rpath(start, argv[4]);
  } else {
    fprintf(stderr, "Invalid argument\n");
  }

  pmesg("Hello world argc: %d", argc);
  return 0;
}

// leftright: 7 (right hand rule), 6 (left hand rule)
// 0 = left, 1 = right, 2 = up/down
int start_border(Map *map, int r, int c, int leftright) {
  // 1.From which side are we accessing the maze
  // 2. in which row are we?

  // From left
  if (c == 1) {
    // Even row
    if (r % 2 == 0) {
      return leftright == RIGHT_RULE ? 2 : 1;
    } else {
      return leftright == RIGHT_RULE ? 1 : 2;
    }
  }

  // From up
  if (r == 1) {
    return leftright == RIGHT_RULE ? 1 : 0;
  }

  // From down
  if (r == map->rows) {
    return leftright == RIGHT_RULE ? 1 : 0;
  }

  // From right
  if (c == map->cols) {
    if (r % 2 == 0) {
      return leftright == RIGHT_RULE ? 0 : 2;
    } else {
      return leftright == RIGHT_RULE ? 2 : 0;
    }
  }
  return 1;
}

unsigned char *rpath(coordinates_t start, char *file_name) {
  // Load file
  FILE *fptr;
  Map maze = {0};
  if (init_maze(&fptr, &maze, file_name) == NULL) {
    return NULL;
  }

  if (map_load(&fptr, &maze) == NULL) {
    return NULL;
  }

  pmesg("Start border: %d",
        start_border(&maze, start.left, start.right, RIGHT_RULE));

  // Search algorithm

  (void)(start);
  return maze.cells;
}

Map *map_load(FILE **fptr, Map *maze) {
  int count = 0;
  int tmp = 0;
  while (fscanf(*fptr, "%d", &tmp) != EOF) {
    if (tmp < 0 || tmp > 255) {
      free(maze->cells);
      return NULL;
    }

    if (count >= (maze->rows * maze->cols)) {
      free(maze->cells);
      return NULL;
    }

    // Save value
    maze->cells[count] = (unsigned char)tmp;
    count++;
  }
  pmesg("count: %d", count);
  return maze;
}

Map *init_maze(FILE **fptr, Map *maze, char *file_name) {

  // Open file
  *fptr = fopen(file_name, "r");
  if (*fptr == NULL) {
    pmesg("Error opening file!");
    return NULL;
  }

  // Get size from first row
  int size[2] = {0};
  for (int i = 0; i < 2; i++) {
    fscanf(*fptr, "%d", &size[i]);
  }

  unsigned char *ptr = malloc(sizeof(unsigned char) * size[0] * size[1]);
  if (ptr == NULL) {
    pmesg("No memory!");
    return NULL;
  }

  maze->rows = size[0];
  maze->cols = size[1];
  maze->cells = ptr;

  for (int i = 0; i < size[0] * size[1]; i++) {
    maze->cells[i] = 0;
  }

  return maze;
}

/* Returns ptr for success, 0 as failure
 * TODO: Split into more functions
 */
unsigned char *maze_test(char *file_name) {
  // Load file
  FILE *fptr;
  Map maze = {0};
  if (init_maze(&fptr, &maze, file_name) == NULL) {
    return NULL;
  }

  if (map_load(&fptr, &maze) == NULL) {
    return NULL;
  }

  pmesg("rows: %d, columns: %d", maze.rows, maze.cols);

  // TODO: into one function
  // Check border validity
  // get neigbourts around one cell
  for (int i = 0; i < maze.rows; i++) {
    for (int j = 0; j < maze.cols; j++) {
      unsigned char neighbors[3];

      // Left
      neighbors[0] = (j > 0) ? maze.cells[(i * maze.cols) + j - 1] : EMPTY_CELL;
      // Right
      neighbors[1] = (j < maze.cols - 1) ? maze.cells[(i * maze.cols) + j + 1]
                                         : EMPTY_CELL;

      // Top/down
      if ((((i * maze.cols) + j) % 2) == 0) {
        neighbors[2] =
            (i > 0) ? maze.cells[(i - 1) * maze.cols + j] : EMPTY_CELL;
      } else {
        neighbors[2] = (i < maze.rows - 1) ? maze.cells[(i + 1) * maze.cols + j]
                                           : EMPTY_CELL;
      }

      if (!valid_borders(maze.cells[i * maze.cols + j], neighbors)) {
        free(maze.cells);
        return NULL;
      }
    }
  }

  fclose(fptr);
  return maze.cells;
}

bool valid_borders(unsigned char cell, unsigned char *neighbors) {
  // Left neighbor
  if (neighbors[0] != EMPTY_CELL &&
      (((cell >> 0) & 1) != ((neighbors[0] >> 1) & 1))) {
    return false;
  }
  // Right neighbor
  if (neighbors[1] != EMPTY_CELL &&
      (((cell >> 1) & 1) != ((neighbors[1] >> 0) & 1))) {
    return false;
  }
  // Up/Down neighbor
  if (neighbors[2] != EMPTY_CELL &&
      (((cell >> 2) & 1) != ((neighbors[2] >> 2) & 1))) {
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
