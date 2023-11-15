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

#define LEFT_BORDER 0
#define RIGHT_BORDER 1
#define VERTICAL_BORDER 2
#define NDIR 3

typedef struct {
  int rows;
  int cols;
  unsigned char *cells;
} Map;

typedef struct {
  int left;
  int right;
} coordinates_t;

int new_border(int old_border);
void next_cell(Map *maze, coordinates_t *curr_cell, int border);
int get_right_border(Map *maze, coordinates_t curr_cell, int border);
bool isborder(Map *map, int r, int c, int border);
bool valid_borders(unsigned char cell, unsigned char *neighbors);
void print_help();
unsigned char *maze_test(char *file_name);
int start_border(Map *map, int r, int c, int leftright);    // TODO
int rpath(coordinates_t start, char *file_name);            // TODO
void lpath(Map *map, coordinates_t start, char *file_name); // TODO
Map *init_maze(FILE **fptr, Map *maze, char *file_name);
void free_maze(FILE **fptr, Map *maze);
Map *map_load(FILE **fptr, Map *maze);

// Index represent current border, value for index represent right border
int odd_dir[3] = {2, 0, 1};
int even_dir[3] = {1, 2, 0};

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
      fprintf(stdout, "Invalid\n");
      return 1;
    } else {
      free(cells);
      fprintf(stdout, "Valid\n");
      return 0;
    }
  } else if (argc == 5 && !strcmp(argv[1], "--rpath")) {
    pmesg("START at: [%s][%s]", argv[2], argv[3]);
    // Zero index
    coordinates_t start = {.left = atoi(argv[2]) - 1,
                           .right = atoi(argv[3]) - 1};
    // TODO Valid start point somehow
    rpath(start, argv[4]);
  } else {
    fprintf(stderr, "Invalid argument\n");
  }
  return 0;
}

// leftright: 7 (right hand rule), 6 (left hand rule)
// 0 = left, 1 = right, 2 = up/down
int start_border(Map *map, int r, int c, int leftright) {

  // From left
  if (c == 0) {
    // Even row
    if (r % 2 == 0) {
      return leftright == RIGHT_RULE ? RIGHT_BORDER : VERTICAL_BORDER;
    } else {
      return leftright == RIGHT_RULE ? VERTICAL_BORDER : RIGHT_BORDER;
    }
  }

  // From up
  if (r == 0) {
    return leftright == RIGHT_RULE ? RIGHT_BORDER : LEFT_BORDER;
  }

  // From down
  if (r == (map->rows - 1)) {
    return leftright == RIGHT_RULE ? RIGHT_BORDER : LEFT_BORDER;
  }

  // From right
  if (c == (map->cols - 1)) {
    if (r % 2 == 0) {
      return leftright == RIGHT_RULE ? VERTICAL_BORDER : LEFT_BORDER;
    } else {
      return leftright == RIGHT_RULE ? LEFT_BORDER : VERTICAL_BORDER;
    }
  }
  return 1;
}

int rpath(coordinates_t start_cell, char *file_name) {
  // Load file
  FILE *fptr;
  Map maze = {0};
  if (init_maze(&fptr, &maze, file_name) == NULL) {
    return -1;
  }

  if (map_load(&fptr, &maze) == NULL) {
    return -1;
  }

  pmesg("Start border: %d",
        start_border(&maze, start_cell.left, start_cell.right, RIGHT_RULE));

  // Zero index
  coordinates_t curr_cell = start_cell;
  int curr_border =
      start_border(&maze, start_cell.left, start_cell.right, RIGHT_RULE);

  // Search algorithm
  while (1) {
    fprintf(stdout, "%d,%d\n", curr_cell.left + 1, curr_cell.right + 1);

    curr_border = get_right_border(&maze, curr_cell, curr_border);

    while (isborder(&maze, curr_cell.left, curr_cell.right, curr_border)) {
      curr_border = get_right_border(&maze, curr_cell, curr_border);
      pmesg("Border in triangle[%d][%d] %d", curr_cell.left + 1,
            curr_cell.right + 1, curr_border);
    }

    next_cell(&maze, &curr_cell, curr_border);
    // Is in maze, could be exit
    if (curr_cell.left < 0 || curr_cell.left > maze.rows - 1 ||
        curr_cell.right < 0 || curr_cell.right > maze.cols - 1) {
      pmesg("We found exit! %d, %d vs %d, %d", curr_cell.left + 1,
            curr_cell.right + 1, maze.rows, maze.cols);
      break;
    }

    pmesg("next cell; %d, %d", curr_cell.left + 1, curr_cell.right + 1);
    curr_border = new_border(curr_border);
    if (curr_border == -1) {
      pmesg("Some border error");
    }
  }

  free_maze(&fptr, &maze);
  return 0;
}

void free_maze(FILE **fptr, Map *maze) {
  fclose(*fptr);
  free(maze->cells);
}

int new_border(int old_border) {
  if (old_border == 1) {
    return 0;
  } else if (old_border == 0) {
    return 1;
  } else if (old_border == 2) {
    return 2;
  }

  return -1;
}

void next_cell(Map *maze, coordinates_t *curr_cell, int border) {
  if ((curr_cell->left * maze->cols + curr_cell->right) % 2 == 0) {
    if (border == 2) {
      curr_cell->left -= 1;
    }
  } else {
    if (border == 2) {
      curr_cell->left += 1;
    }
  }

  if (border == 1) {
    curr_cell->right += 1;
  } else if (border == 0) {
    curr_cell->right -= 1;
  }
}

int get_right_border(Map *maze, coordinates_t curr_cell, int border) {
  if ((curr_cell.left * maze->cols + curr_cell.right) % 2 == 0) {
    pmesg("TESTING even");
    return even_dir[border];
  } else {
    pmesg("TESTING odd");
    return odd_dir[border];
  }

  return -1;
}

// 0*2^0 + 1*2^1 + 0*2^2 = 2
// 0*2^0 + 0*2^1 + 1*2^2 = 4
// 1*2^0 + 0*2^1 + 1*2^2 = 5
// leva  + prava + spodni
// 0			 1  		 2
bool isborder(Map *map, int r, int c, int border) {
  return (map->cells[r * map->cols + c] >> border) & 1 ? true : false;
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

  if (count != (maze->rows * maze->cols)) {
    free(maze->cells);
    return NULL;
  }

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

void print_help() {
  char *help = "maze [command] <options>\n\
\n\
--test <file> check, if given file has needed maze definition. If the maze file is valid program pritns Valid. Otherwise prints Invalid\n\
--rpath <R> <C> tries to find path in maze on entry on column R and row C. Path is searched by rule of right hand\n\
--lpath <R> <C> trues to find path in maze on entry on column R and row C. Path is searched by the rule of left hand";

  printf("%s", help);
}
