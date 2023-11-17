/**
 * @file maze.c
 *
 * @author  Spac Petr <xspacpe00@stud.fit.vutbr.cz>
 * @date 2023-11
 *
 */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// TODO: REMOVE THIS TODO IN PRODUCTION CODE
#ifndef NDEBUG
#define pmesg(s, ...)                                                          \
  fprintf(stderr, "%s:%u: " s "\n", __func__, __LINE__, ##__VA_ARGS__)
#else
#define pmesg(s, ...)                                                          \
  do {                                                                         \
  } while (0)
#endif

#define CELL_NEIGHBORS_COUNT 3
#define EMPTY_CELL 10

#define RIGHT_HAND 6
#define LEFT_HAND 7

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
  int row;
  int col;
} coordinates_t;

int new_border(int old_border);
bool is_exit(Map *maze, coordinates_t cell);
void next_cell(Map *maze, coordinates_t *curr_cell, int border);
int get_side_border(Map *maze, coordinates_t curr_cell, int border,
                    int leftright);
bool isborder(Map *map, int r, int c, int border);
bool valid_borders(unsigned char cell, unsigned char *neighbors);
void print_help();
unsigned char *maze_test(char *file_name);
int start_border(Map *map, int r, int c, int leftright);
int path_by_rule(coordinates_t start, char *file_name, int leftright);
Map *init_maze(FILE **fptr, Map *maze, char *file_name);
void free_maze(FILE **fptr, Map *maze);
Map *map_load(FILE **fptr, Map *maze);

// TODO Clean code, make more redeable in general

// Array index represents current border, value for index represents right
// border from current border
int rpath_even_dir[NDIR] = {2, 0, 1};
int rpath_odd_dir[NDIR] = {1, 2, 0};

int lpath_even_dir[NDIR] = {1, 2, 0};
int lpath_odd_dir[NDIR] = {2, 0, 1};

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
      return 0;
    } else {
      free(cells);
      fprintf(stdout, "Valid\n");
      return 0;
    }
  } else if (argc == 5 && !strcmp(argv[1], "--rpath")) {
    coordinates_t start = {.row = atoi(argv[2]), .col = atoi(argv[3])};
    // TODO Valid start point somehow
    int result = path_by_rule(start, argv[4], RIGHT_HAND);
    if (result == -1) {
      return 1;
    }
  } else if (argc == 5 && !strcmp(argv[1], "--lpath")) {
    pmesg("START at: [%s][%s]", argv[2], argv[3]);
    coordinates_t start = {.row = atoi(argv[2]), .col = atoi(argv[3])};
    // TODO Valid start point somehow
    int result = path_by_rule(start, argv[4], LEFT_HAND);
    if (result == -1) {
      return 1;
    }
  } else {
    fprintf(stderr, "Invalid argument\n");
  }
  return 0;
}

// leftright: 7 (right hand rule), 6 (left hand rule)
// 0 = left, 1 = right, 2 = up/down
int start_border(Map *map, int r, int c, int leftright) {

  // From left
  if (c == 1) {
    // Even row
    if (r % 2 != 0) {
      return leftright == RIGHT_HAND ? RIGHT_BORDER : VERTICAL_BORDER;
    } else {
      return leftright == RIGHT_HAND ? VERTICAL_BORDER : RIGHT_BORDER;
    }
  }

  // From up
  if (r == 1) {
    return leftright == RIGHT_HAND ? LEFT_BORDER : RIGHT_BORDER;
  }

  // From down
  if (r == map->rows) {
    return leftright == RIGHT_HAND ? RIGHT_BORDER : LEFT_BORDER;
  }

  // From right
  if (c == map->cols) {
    if (r % 2 != 0) {
      return leftright == RIGHT_HAND ? LEFT_BORDER : VERTICAL_BORDER;
    } else {
      return leftright == RIGHT_HAND ? VERTICAL_BORDER : LEFT_BORDER;
    }
  }
  return 1;
}

int path_by_rule(coordinates_t start_cell, char *file_name, int leftright) {
  // Load file
  FILE *fptr;
  Map maze = {0};
  if (init_maze(&fptr, &maze, file_name) == NULL) {
    fprintf(stderr, "Map did not initialize!\n");
    return -1;
  }

  if (map_load(&fptr, &maze) == NULL) {
    fprintf(stderr, "Map did not load!\n");
    return -1;
  }

  coordinates_t curr_cell = start_cell;
  int curr_border =
      start_border(&maze, start_cell.row, start_cell.col, leftright);

  // Search algorithm
  while (1) {
    fprintf(stdout, "%d,%d\n", curr_cell.row, curr_cell.col);

    while (isborder(&maze, curr_cell.row, curr_cell.col, curr_border)) {
      curr_border = get_side_border(&maze, curr_cell, curr_border, leftright);
    }

    next_cell(&maze, &curr_cell, curr_border);

    if (is_exit(&maze, curr_cell)) {
      pmesg("We found exit! %d, %d vs %d, %d", curr_cell.row, curr_cell.col,
            maze.rows, maze.cols);
      break;
    }

    pmesg("next cell; %d, %d", curr_cell.row, curr_cell.col);
    pmesg("curr border; %d", curr_border);
    curr_border =
        get_side_border(&maze, curr_cell, new_border(curr_border), leftright);
    pmesg("new cell border; %d", curr_border);
    if (curr_border == -1) {
      pmesg("Some border error");
      return -1;
    }
  }

  free_maze(&fptr, &maze);
  return 0;
}

bool is_exit(Map *maze, coordinates_t cell) {
  if (cell.row < 1 || cell.row > maze->rows || cell.col < 1 ||
      cell.col > maze->cols) {
    pmesg("We found exit! cell:%d, %d ??? %d, %d", cell.row, cell.col,
          maze->rows, maze->cols);
    return true;
  }
  return false;
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
  bool isMazeColEven = (maze->cols % 2) == 0;
  bool isRowEven = (curr_cell->row % 2) == 0;

  int cellIndex =
      ((curr_cell->row - 1) * maze->cols + (curr_cell->col - 1)) + 1;
  bool isCellIndexEven = ((cellIndex) % 2 == 0);

  if (border == 2) {
    if (isMazeColEven && isRowEven) {
      curr_cell->row += isCellIndexEven ? -1 : 1;
    } else {
      curr_cell->row += isCellIndexEven ? 1 : -1;
    }
  } else if (border == 1) {
    curr_cell->col += 1;
  } else if (border == 0) {
    curr_cell->col -= 1;
  }
}

int get_side_border(Map *maze, coordinates_t curr_cell, int border,
                    int leftright) {
  bool isMazeColEven = (maze->cols % 2) == 0;
  bool isRowEven = (curr_cell.row % 2) == 0;

  // + 1 to remove 0 indexing
  int cellIndex = ((curr_cell.row - 1) * maze->cols + (curr_cell.col - 1)) + 1;
  bool isCellIndexEven = ((cellIndex) % 2 == 0);

  // When maze has even cols, triangles are changing direction on every even row
  if (isMazeColEven && isRowEven) {
    if (isCellIndexEven) {
      return leftright == LEFT_HAND ? lpath_odd_dir[border]
                                    : rpath_odd_dir[border];
    } else {
      return leftright == LEFT_HAND ? lpath_even_dir[border]
                                    : rpath_even_dir[border];
    }
  } else {
    if (isCellIndexEven) {
      return leftright == LEFT_HAND ? lpath_even_dir[border]
                                    : rpath_even_dir[border];
    } else {
      return leftright == LEFT_HAND ? lpath_odd_dir[border]
                                    : rpath_odd_dir[border];
    }
  }

  return -1;
}

// 0*2^0 + 1*2^1 + 0*2^2 = 2
// 0*2^0 + 0*2^1 + 1*2^2 = 4
// 1*2^0 + 0*2^1 + 1*2^2 = 5
// leva  + prava + spodni
// 0			 1  		 2
bool isborder(Map *map, int r, int c, int border) {
  // Do not forget on 0 index
  return (map->cells[(r - 1) * map->cols + (c - 1)] >> border) & 1 ? true
                                                                   : false;
}

Map *map_load(FILE **fptr, Map *maze) {
  int count = 1;
  int tmp = 0;
  while (fscanf(*fptr, "%d", &tmp) != EOF) {
    if (tmp < 0 || tmp > 255) {
      free(maze->cells);
      return NULL;
    }

    if (count > (maze->rows * maze->cols)) {
      free(maze->cells);
      return NULL;
    }

    // Save value
    maze->cells[count - 1] = (unsigned char)tmp;
    count++;
  }

  if (count != (maze->rows * maze->cols + 1)) {
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
    fclose(fptr);
    return NULL;
  }

  pmesg("rows: %d, columns: %d", maze.rows, maze.cols);

  // TODO: into one function
  // Check border validity
  // get neigbourts around one cell
  for (int row = 1; row <= maze.rows; row++) {
    for (int col = 1; col <= maze.cols; col++) {
      unsigned char neighbors[CELL_NEIGHBORS_COUNT];

      int cellIndex = (row - 1) * maze.cols + (col - 1);
      bool isCellIndexEven = (cellIndex % 2) == 0;
      bool isMazeColEven = (maze.cols % 2) == 0;
      bool isCurrRowEven = (row % 2) == 0;

      // Left
      neighbors[0] = (col > 1) ? maze.cells[cellIndex - 1] : EMPTY_CELL;
      // Right
      neighbors[1] = (col < maze.cols) ? maze.cells[cellIndex + 1] : EMPTY_CELL;

      //
      // TODO: SPlit into another function
      if (isMazeColEven && isCurrRowEven) {
        if (isCellIndexEven) {
          neighbors[2] = (row < maze.rows) ? maze.cells[cellIndex + maze.cols]
                                           : EMPTY_CELL;
        } else {
          neighbors[2] =
              (row > 1) ? maze.cells[cellIndex - maze.cols] : EMPTY_CELL;
        }
      } else {
        if (isCellIndexEven) {
          neighbors[2] =
              (row > 1) ? maze.cells[cellIndex - maze.cols] : EMPTY_CELL;
        } else {
          neighbors[2] = (row < maze.rows) ? maze.cells[cellIndex + maze.cols]
                                           : EMPTY_CELL;
        }
      }

      if (!valid_borders(maze.cells[(row - 1) * maze.cols + (col - 1)],
                         neighbors)) {
        free(maze.cells);
        return NULL;
      }
    }
  }

  fclose(fptr);
  return maze.cells;
}

bool valid_borders(unsigned char cell, unsigned char *neighbors) {
  pmesg("cell %d", cell);
  pmesg("left %d", neighbors[0]);
  pmesg("right %d", neighbors[1]);
  pmesg("up/down %d", neighbors[2]);
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
