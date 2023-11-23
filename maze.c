/**
 * @file maze.c
 *
 * @author  Spac Petr <xspacpe00@stud.fit.vutbr.cz>
 * @date 2023-11
 *
 */
#include <assert.h>
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

// Queue
struct node {
  coordinates_t cell;
  struct node *parent;
  struct node *next;
};

struct node *front = NULL;
struct node *rear = NULL;

typedef struct {
  struct node *array;
  size_t used;
  size_t size;
} Array;

void init_array(Array *a, size_t init_size) {
  a->array = malloc(init_size * sizeof(struct node));
  a->used = 0;
  a->size = init_size;
}

void array_insert(Array *arr, struct node element) {
  // a->used is the number of used entries, because a->array[a->used++] updates
  // a->used only *after* the array has been accessed. Therefore a->used can go
  // up to a->size
  if (arr->used == arr->size) {
    arr->size *= 2;
    arr->array = realloc(arr->array, arr->size * sizeof(struct node));
  }
  arr->array[arr->used++] = element;
}

void free_array(Array *a) {
  free(a->array);
  a->array = NULL;
  a->used = a->size = 0;
}

void enqueue(coordinates_t cell, struct node *parent);
void dequeue(struct node *new_cell);

int new_border(int old_border);
int start_border(Map *map, int r, int c, int leftright);
int get_side_border(Map *maze, coordinates_t curr_cell, int border,
                    int leftright);
bool is_valid_start(Map *maze, int r, int c);
bool isborder(Map *map, int r, int c, int border);
bool valid_borders(unsigned char cell, unsigned char *neighbors);

int get_cell_index(Map *maze, int r, int c);
bool is_exit(Map *maze, coordinates_t cell);
void next_cell(Map *maze, coordinates_t *curr_cell, int border);
bool is_in_maze(Map *maze, coordinates_t point);
void print_help();

int path_by_rule(coordinates_t start_cell, char *file_name, int leftright);
int bfs(coordinates_t start_cell, char *file_name);

Map *init_maze(FILE **fptr, Map *maze, char *file_name);
void free_maze(FILE **fptr, Map *maze);
Map *map_load(FILE **fptr, Map *maze);
unsigned char *maze_test(char *file_name);

// TODO Clean code, make more redeable in general

// Array index represents current border, value for index represents right
// border from current border
const int rpath_even_dir[NDIR] = {2, 0, 1};
const int rpath_odd_dir[NDIR] = {1, 2, 0};

const int lpath_even_dir[NDIR] = {1, 2, 0};
const int lpath_odd_dir[NDIR] = {2, 0, 1};

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
      return 0;
    } else {
      free(cells);
      fprintf(stdout, "Valid\n");
      return 0;
    }
  } else if (argc == 5 && !strcmp(argv[1], "--rpath")) {
    coordinates_t start = {.row = atoi(argv[2]), .col = atoi(argv[3])};

    int result = path_by_rule(start, argv[4], RIGHT_HAND);
    if (result == -1) {
      return 1;
    }
  } else if (argc == 5 && !strcmp(argv[1], "--lpath")) {
    pmesg("START at: [%s][%s]", argv[2], argv[3]);
    coordinates_t start = {.row = atoi(argv[2]), .col = atoi(argv[3])};

    int result = path_by_rule(start, argv[4], LEFT_HAND);
    if (result == -1) {
      return 1;
    }
  } else if (argc == 5 && !strcmp(argv[1], "--shortest")) {
    pmesg("START at: [%s][%s]", argv[2], argv[3]);
    coordinates_t start = {.row = atoi(argv[2]), .col = atoi(argv[3])};

    int result = bfs(start, argv[4]);
    if (result == -1) {
      return 1;
    }
  } else {
    fprintf(stderr, "Invalid argument!\n");
  }
  return 0;
}

bool is_valid_start(Map *maze, int r, int c) {
  bool is_maze_col_even = (maze->cols % 2) == 0;
  bool is_cell_row_even = (r % 2) == 0;
  bool is_cell_index_even = get_cell_index(maze, r, c) % 2 == 0;

  // From left
  if (c == 1 && !isborder(maze, r, c, 0)) {
    return true;
  }

  // From up
  if (r == 1) {
    if ((c % 2 != 0) && !isborder(maze, r, c, 2)) {
      return true;
    }
  }

  // From down
  if (r == maze->rows) {
    if (is_maze_col_even && is_cell_row_even) {
      if (!is_cell_index_even && !isborder(maze, r, c, 2)) {
        return true;
      }
    } else {
      if (is_cell_index_even && !isborder(maze, r, c, 2)) {
        return true;
      }
    }
  }
  // From right
  if (c == maze->cols && !isborder(maze, r, c, 1)) {
    return true;
  }

  return false;
}

bool is_in_maze(Map *maze, coordinates_t cell) {
  pmesg("CELL r: %d, c: %d", cell.row, cell.col);

  if (cell.row > 0 && cell.row <= maze->rows && cell.col > 0 &&
      cell.col <= maze->cols) {
    pmesg("Is in maze");
    return true;
  }
  return false;
}

void enqueue(coordinates_t cell, struct node *parent) {
  pmesg("engquing %d, %d", cell.row, cell.col);
  struct node *nptr = malloc(sizeof(struct node));
  assert(nptr != NULL);
  if (nptr == NULL) {
    fprintf(stderr, "Failed to malloc\n");
  }

  nptr->cell = cell;
  nptr->next = NULL;
  nptr->parent = parent;

  if (rear == NULL) {
    front = nptr;
    rear = nptr;
  } else {
    rear->next = nptr;
    rear = rear->next;
  }
}

void display() {
  struct node *temp;
  temp = front;
  pmesg("DISPLAY:");
  while (temp != NULL) {
    pmesg("%d, %d\t", temp->cell.row, temp->cell.col);
    temp = temp->next;
  }
  pmesg("END DISPLAY");
}

void freeQueue() {
  while (front != NULL) {
    struct node *temp = front;
    front = front->next;
    free(temp);
  }
  rear = NULL; // Set rear to NULL as the queue is now empty
}

void dequeue(struct node *node) {
  if (front == NULL) {
    printf("\n\nqueue is empty \n");
    front = NULL;
    rear = NULL;
  } else {
    struct node *temp;
    temp = front;
    *node = *temp;

    if (front == rear) {
      front = front->next;
      rear = rear->next;
    } else {
      front = front->next;
    }

    pmesg("DELETE: %d, %d", temp->cell.row, temp->cell.col);
    free(temp);
  }
}

int get_cell_index(Map *maze, int r, int c) {
  return ((r - 1) * maze->cols + (c - 1)) + 1;
}

int is_lebeled(Map *maze, coordinates_t cell) {
  // -1 because we need straight access to cell
  int cell_index = get_cell_index(maze, cell.row, cell.col) - 1;
  // showbits(maze->cells[cell_index]);
  if ((maze->cells[cell_index] >> 7) & 1) {
    return true;
  }
  return false;
}

void set_label(Map *maze, coordinates_t cell) {
  int cell_index = get_cell_index(maze, cell.row, cell.col) - 1;
  maze->cells[cell_index] = maze->cells[cell_index] | 128;
}

bool is_goal(Map *maze, coordinates_t start, coordinates_t new_cell,
             coordinates_t base_cell) {
  // Return false immediately if the current position is the start position,
  // or if the current position is outside the bounds of the maze.
  if ((new_cell.row < 1 || new_cell.row > maze->rows || new_cell.col < 1 ||
       new_cell.col > maze->cols) &&
      (base_cell.row != start.row || base_cell.col != start.col)) {
    return true;
  }
  return false;
}

void reconstruct_path(coordinates_t s, struct node e) {
  struct node *current = &e;
  Array path;
  init_array(&path, 10);

  // Trace back from the end node to the start node
  while (current != NULL) {
    array_insert(&path, *current);
    current = current->parent;
  }

  if (path.array[path.used - 1].cell.row == s.row &&
      path.array[path.used - 1].cell.col == s.col) {
    // Display the path in reverse (from start to end)
    for (int i = path.used - 1; i >= 0; i--) {
      fprintf(stdout, "%d,%d\n", path.array[i].cell.row,
              path.array[i].cell.col);
    }
  } else {
    fprintf(stdout, "%d,%d\n", s.row, s.col);
  }

  free_array(&path);
}

bool check_neighbours(Map *maze, coordinates_t start,
                      struct node *dequeued_node, Array *visited, int *count) {

  // Search visited for node and its index
  struct node *lastVisited = &visited->array[*count - 1];
  for (int k = 0; k < *count; k++) {
    if (visited->array[k].cell.row == dequeued_node->cell.row &&
        visited->array[k].cell.col == dequeued_node->cell.col) {
      lastVisited = &visited->array[k];
    }
  }

  pmesg("Checking cell %d %d", dequeued_node->cell.row,
        dequeued_node->cell.col);
  pmesg("LAST visited: %d %d", lastVisited->cell.row, lastVisited->cell.col);

  for (int i = 0; i < CELL_NEIGHBORS_COUNT; i++) {
    if (!isborder(maze, dequeued_node->cell.row, dequeued_node->cell.col, i)) {
      pmesg("border %d is free", i);

      struct node tmp = *dequeued_node;
      next_cell(maze, &tmp.cell, i);

      pmesg("CHECKING CELL %d %d", tmp.cell.row, tmp.cell.col);
      if (is_goal(maze, start, tmp.cell, dequeued_node->cell)) {
        pmesg("I WAS CALLED");
        return false;
      }

      if (is_in_maze(maze, tmp.cell)) {

        if (!is_lebeled(maze, tmp.cell)) {
          set_label(maze, tmp.cell);
          tmp.parent = lastVisited;
          array_insert(visited, tmp);

          enqueue(tmp.cell, lastVisited);
          *count += 1;
        }
      }
    }
  }
  pmesg("\n");
  pmesg("\n");
  return true;
}

int bfs(coordinates_t start, char *file_name) {
  // Load file
  FILE *fptr;
  Map maze = {0};

  if (init_maze(&fptr, &maze, file_name) == NULL) {
    fprintf(stderr, "Map did not initialize!\n");
    return -1;
  }

  if (map_load(&fptr, &maze) == NULL) {
    fprintf(stderr, "Map did not load!\n");
    fclose(fptr);
    return -1;
  }

  if (!is_valid_start(&maze, start.row, start.col)) {
    fprintf(stderr, "Invalid entry cell!\n");
    free_maze(&fptr, &maze);
    return -1;
  }

  Array visited;
  init_array(&visited, maze.cols * maze.rows);

  struct node end = {0};
  struct node root = {.cell = start, .parent = NULL, .next = NULL};
  enqueue(start, NULL);

  int j = 0;
  set_label(&maze, start);

  array_insert(&visited, root);
  j++;

  while (front != NULL) {
    struct node dequeued_node;
    dequeue(&dequeued_node);
    display();

    if (!check_neighbours(&maze, root.cell, &dequeued_node, &visited, &j)) {
      end = dequeued_node;
      break;
    };
  }

  reconstruct_path(start, end);

  freeQueue();
  free_maze(&fptr, &maze);
  free_array(&visited);
  return 0;
}

// leftright: 7 (right hand rule), 6 (left hand rule)
// 0 = left, 1 = right, 2 = up/down
int start_border(Map *map, int r, int c, int leftright) {

  // From left
  if (c == 1 && !isborder(map, r, c, 0)) {
    // Even row
    if (r % 2 != 0) {
      pmesg("pls %d", leftright);
      return leftright == RIGHT_HAND ? RIGHT_BORDER : VERTICAL_BORDER;
    } else {
      pmesg("stafdsfdsft %d", leftright);
      return leftright == RIGHT_HAND ? VERTICAL_BORDER : RIGHT_BORDER;
    }
  }

  // From up
  if (r == 1 && !isborder(map, r, c, 2)) {
    pmesg("lul %d", leftright);
    return leftright == RIGHT_HAND ? LEFT_BORDER : RIGHT_BORDER;
  }

  // From down
  if (r == map->rows && !isborder(map, r, c, 2)) {
    pmesg("test %d", leftright);
    return leftright == RIGHT_HAND ? RIGHT_BORDER : LEFT_BORDER;
  }

  // From right
  if (c == map->cols && !isborder(map, r, c, 1)) {
    if (r % 2 != 0) {
      pmesg("xd %d", leftright);
      return leftright == RIGHT_HAND ? LEFT_BORDER : VERTICAL_BORDER;
    } else {
      pmesg("123 %d", leftright);
      return leftright == RIGHT_HAND ? VERTICAL_BORDER : LEFT_BORDER;
    }
  }
  pmesg("pepa %d", leftright);
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
    fclose(fptr);
    return -1;
  }

  if (!is_valid_start(&maze, start_cell.row, start_cell.col)) {
    fprintf(stderr, "Invalid entry cell!\n");
    free_maze(&fptr, &maze);
    return -1;
  }

  coordinates_t curr_cell = start_cell;
  int curr_border =
      start_border(&maze, start_cell.row, start_cell.col, leftright);
  pmesg("start %d", curr_border);

  // Search algorithm
  int count = 0;
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

    pmesg("curr border %d; for cell %d %d", curr_border, curr_cell.row,
          curr_cell.col);
    curr_border =
        get_side_border(&maze, curr_cell, new_border(curr_border), leftright);
    pmesg("new cell border; %d", curr_border);
    if (curr_border == -1) {
      pmesg("Some border error");
      return -1;
    }
    count++;
  }

  free_maze(&fptr, &maze);
  return 0;
}

bool is_exit(Map *maze, coordinates_t cell) {
  if ((cell.row < 1 || cell.row > maze->rows || cell.col < 1 ||
       cell.col > maze->cols)) {
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
  pmesg("TESTTSET %d", cellIndex);
  // if (curr_cell->row - 1 < 1 || curr_cell->row + 1 > maze->rows) {
  //   return;
  // }

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
  // pmesg("%d %d", r, c);
  // Do not forget on 0 index
  // showbits(map->cells[(r - 1) * map->cols + (c - 1)]);
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
    pmesg("SIZE: %d", size[i]);
  }

  unsigned char *ptr = malloc(sizeof(unsigned char) * size[0] * size[1]);
  if (ptr == NULL) {
    pmesg("No memory!");
    fclose(*fptr);
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

      // For accessing maze.cells we need 0 index
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
  pmesg("Cell VALUE: %d", cell);
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
