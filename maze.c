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

//
// leva  + prava + spodni
// 0			 1  		 2

// TODO: REMOVE THIS TODO IN PRODUCTION CODE
#ifndef NDEBUG
#define pmesg(s, ...)                                                          \
  fprintf(stderr, "%s:%u: " s "\n", __func__, __LINE__, ##__VA_ARGS__)
#else
#define pmesg(s, ...)                                                          \
  do {                                                                         \
  } while (0)
#endif

#define CELL_NEIGHBORS_COUNT 3 ///< Number of neighbours each cell has.
#define EMPTY_CELL 10          ///< This represents non existing cell

#define RIGHT_HAND 6
#define LEFT_HAND 7

#define LEFT_BORDER 0
#define RIGHT_BORDER 1
#define VERTICAL_BORDER 2

#define NDIR 3 ///< Number of possible directions from cell

/**
 * @brief Map represents matrix
 *
 */
typedef struct {
  int rows;
  int cols;
  unsigned char *cells;
} Map;

/**
 * @brief Coordinates for cell
 *
 */
typedef struct {
  int row;
  int col;
} coordinates_t;

/**
 * @brief Node represents an item in Queue
 *
 */
struct node {
  coordinates_t cell;
  struct node *parent;
  struct node *next;
};

/// Front of the Queue
struct node *front = NULL;
/// Rear of the Queue
struct node *rear = NULL;

/**
 * Array used in BFS
 *
 */
typedef struct {
  struct node *array;
  size_t used;
  size_t size;
} Array;

/**
 * Allocates memory for an array and initializes its metadata.
 * @param arr Pointer to the Array structure to be initialized.
 * @param init_size The initial number of elements.
 * @return 0 on success, non-zero on allocation failure.
 */
int init_array(Array *arr, size_t init_size) {
  arr->array = malloc(init_size * sizeof(struct node));
  if (arr->array == NULL) {
    return -1;
  }
  arr->used = 0;
  arr->size = init_size;
  return 0;
}

/**
 * Insert item into array.
 * @param arr Pointer to array
 * @param init_size Initial size of the array
 */
void array_insert(Array *arr, struct node item) {

  if (arr->used == arr->size) {
    arr->size *= 2;
    arr->array = realloc(arr->array, arr->size * sizeof(struct node));
    if (arr->array == NULL) {
      return;
    }
  }
  arr->array[arr->used++] = item;
}

/**
 * Free an array.
 * @param arr Pointer to array
 */
void free_array(Array *arr) {
  free(arr->array);
  arr->array = NULL;
  arr->used = arr->size = 0;
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

bool is_lebeled(Map *maze, coordinates_t cell);
void set_label(Map *maze, coordinates_t cell);
bool is_goal(Map *maze, coordinates_t start, coordinates_t new_cell,
             coordinates_t base_cell);

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

// Array index represents current border, value for index represents right
// border from current border
const int rpath_even_dir[NDIR] = {2, 0, 1};
const int rpath_odd_dir[NDIR] = {1, 2, 0};

const int lpath_even_dir[NDIR] = {1, 2, 0};
const int lpath_odd_dir[NDIR] = {2, 0, 1};

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

/**
 * Validate starting cell
 * @param maze Pointer to loaded maze
 * @param r cell row
 * @param c cell column
 * @returns True as valid, false as invalid
 */
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

/**
 * Add cell to queue.
 * @param cell coordinations of cell
 * @param parent parent cell
 */
void enqueue(coordinates_t cell, struct node *parent) {
  pmesg("engquing %d, %d", cell.row, cell.col);
  struct node *nptr = malloc(sizeof(struct node));
  assert(nptr != NULL);
  if (nptr == NULL) {
    fprintf(stderr, "Failed to malloc\n");
    return;
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

/**
 * Free queue
 */
void freeQueue() {
  while (front != NULL) {
    struct node *temp = front;
    front = front->next;
    free(temp);
  }
  rear = NULL; // Set rear to NULL as the queue is now empty
}

/**
 * Dequeue node from queue.
 * @param node pointer to a node, which will be dequeued
 */
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

/**
 * Returns 1-based index for given cell.
 * @param maze pointer to loaded maze
 * @param r Row index of the cell (1-based).
 * @param c Column index of the cell (1-based).
 * @return cell index in maze
 */
int get_cell_index(Map *maze, int r, int c) {
  // +1 to remove zero indexing
  return ((r - 1) * maze->cols + (c - 1)) + 1;
}

/**
 * Checks if is cell labeled
 * @param maze pointer to loaded maze
 * @param cell coordinate of given cell
 * @return true as labeled, false as not labeled
 */
bool is_lebeled(Map *maze, coordinates_t cell) {
  // -1 because we need straight access to cell
  int cell_index = get_cell_index(maze, cell.row, cell.col) - 1;
  if ((maze->cells[cell_index] >> 7) & 1) {
    return true;
  }
  return false;
}

/**
 * Label given cell
 * @param maze pointer to loaded maze
 * @param cell coordinate of given cell
 */
void set_label(Map *maze, coordinates_t cell) {
  int cell_index = get_cell_index(maze, cell.row, cell.col) - 1;
  maze->cells[cell_index] = maze->cells[cell_index] | 128;
}

/**
 * Check if cell is exit (used in BFS)
 * @param maze pointer to loaded maze
 * @param start coordinates of start cell
 * @param base_cell coordinates of current_cell
 * @param new_cell current_cells neighbours coordinates
 * @return true for exit, otherwise false
 */
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

/**
 * Reconstructs path from visited cells in BFS
 * @param s coordinates of start
 * @param e end node
 */
void reconstruct_path(coordinates_t s, struct node e) {
  struct node *current = &e;
  Array path;
  if (init_array(&path, 20) == -1) {
    return;
  }

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

/**
 * BFS neighbour checking
 * @param maze pointer to loaded maze
 * @param start coordinates of start
 * @param duqueued_node
 * @param visited pointer to array of visited nodes
 */
bool check_neighbours(Map *maze, coordinates_t start,
                      struct node *dequeued_node, Array *visited) {

  // Search visited for node and its index
  struct node *lastVisited = &visited->array[visited->used - 1];
  for (int k = 0; k < (int)visited->used; k++) {
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
        return false;
      }

      if (is_in_maze(maze, tmp.cell)) {
        if (!is_lebeled(maze, tmp.cell)) {
          set_label(maze, tmp.cell);
          tmp.parent = lastVisited;
          array_insert(visited, tmp);

          enqueue(tmp.cell, lastVisited);
        }
      }
    }
  }
  return true;
}

/**
 * BFS algorithm (https://en.wikipedia.org/wiki/Breadth-first_search)
 * @param start coordinates for start cell
 * @param file_name name of the maze file
 * @return -1 for failure, 0 for success
 */
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
  if (init_array(&visited, maze.cols * maze.rows) == -1) {
    return -1;
  }

  struct node end = {0};
  struct node root = {.cell = start, .parent = NULL, .next = NULL};
  enqueue(start, NULL);

  set_label(&maze, start);

  array_insert(&visited, root);
  pmesg("used %zu, size %zu", visited.used, visited.size);

  while (front != NULL) {
    struct node dequeued_node;
    dequeue(&dequeued_node);
    display();

    if (!check_neighbours(&maze, root.cell, &dequeued_node, &visited)) {
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

/**
 * @param map pointer to a map
 * @param r Row index of the cell (1-based).
 * @param c Column index of the cell (1-based).
 * @param leftright Hand rule (7 left, 6 right)
 * @return 0 as left, 1 as right, 2 as up/down
 */
int start_border(Map *map, int r, int c, int leftright) {

  // From left
  if (c == 1 && !isborder(map, r, c, 0)) {
    // Even row
    if (r % 2 != 0) {
      return leftright == RIGHT_HAND ? RIGHT_BORDER : VERTICAL_BORDER;
    } else {
      return leftright == RIGHT_HAND ? VERTICAL_BORDER : RIGHT_BORDER;
    }
  }

  // From up
  if (r == 1 && !isborder(map, r, c, 2)) {
    return leftright == RIGHT_HAND ? LEFT_BORDER : RIGHT_BORDER;
  }

  // From down
  if (r == map->rows && !isborder(map, r, c, 2)) {
    return leftright == RIGHT_HAND ? RIGHT_BORDER : LEFT_BORDER;
  }

  // From right
  if (c == map->cols && !isborder(map, r, c, 1)) {
    if (r % 2 != 0) {
      return leftright == RIGHT_HAND ? LEFT_BORDER : VERTICAL_BORDER;
    } else {
      return leftright == RIGHT_HAND ? VERTICAL_BORDER : LEFT_BORDER;
    }
  }

  return -1;
}

/**
 * Path finding algorithm
 * @param start_cell coordinates of start
 * @param file_name name of the file with maze
 * @param leftright specifies path finding rule (7 left, 6 right)
 * @return 0 for left, 1 for right, 2 for up/down
 */
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

  if (curr_border == -1) {
    fprintf(stderr, "Invalid entry cell!\n");
    free_maze(&fptr, &maze);
    return -1;
  }

  pmesg("start %d", curr_border);

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

    pmesg("curr border %d; for cell %d %d", curr_border, curr_cell.row,
          curr_cell.col);
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

/**
 * Used in path_by_rule
 * @param fptr pointer on file pointer
 * @param maze pointer to map
 */
bool is_exit(Map *maze, coordinates_t cell) {
  if ((cell.row < 1 || cell.row > maze->rows || cell.col < 1 ||
       cell.col > maze->cols)) {
    pmesg("We found exit! cell:%d, %d ??? %d, %d", cell.row, cell.col,
          maze->rows, maze->cols);
    return true;
  }
  return false;
}

/**
 * @param fptr pointer on file pointer
 * @param maze pointer to map
 */
void free_maze(FILE **fptr, Map *maze) {
  fclose(*fptr);
  free(maze->cells);
}

/**
 * Returns border in newly visited cell
 * @param old_boreder number representing border
 * @return new border
 */
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

/**
 * Returns border in new cell
 * @param maze pointer to map
 * @param curr_cell pointer coordinates of currently searched cell
 * @param border currently searched border
 */
void next_cell(Map *maze, coordinates_t *curr_cell, int border) {
  bool isMazeColEven = (maze->cols % 2) == 0;
  bool isRowEven = (curr_cell->row % 2) == 0;
  int cellIndex = get_cell_index(maze, curr_cell->row, curr_cell->col);
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

/**
 * Returns the next border(which should be checked)
 * @param maze pointer to map
 * @param curr_cell pointer coordinates of currently searched cell
 * @param border currently searched border
 * @return number representing a border
 */
int get_side_border(Map *map, coordinates_t curr_cell, int border,
                    int leftright) {
  bool isMazeColEven = (map->cols % 2) == 0;
  bool isRowEven = (curr_cell.row % 2) == 0;

  int cellIndex = get_cell_index(map, curr_cell.row, curr_cell.col);
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

/**
 * Determines if a specified side of a cell is a border.
 * @param map Pointer to the map structure containing the maze.
 * @param r Row index of the cell (1-based).
 * @param c Column index of the cell (1-based).
 * @param border The specific border side to check (0 left, 1 right, 2 up/down).
 * @return true if the specified side is a border, false otherwise.
 */
bool isborder(Map *map, int r, int c, int border) {
  // Do not forget on 0 indexing
  return (map->cells[(r - 1) * map->cols + (c - 1)] >> border) & 1 ? true
                                                                   : false;
}

/**
 * Loads maze into map struct
 * @param fptr Pointer to the file pointer for reading the maze.
 * @param map Pointer to the maze structure to be loaded.
 * @return pointer to map struct, NULL as failure
 */
Map *map_load(FILE **fptr, Map *map) {
  int count = 1;
  int tmp = 0;
  while (fscanf(*fptr, "%d", &tmp) != EOF) {
    if (tmp < 0 || tmp > 255) {
      free(map->cells);
      return NULL;
    }

    if (count > (map->rows * map->cols)) {
      free(map->cells);
      return NULL;
    }

    // Save value
    map->cells[count - 1] = (unsigned char)tmp;
    count++;
  }

  if (count != (map->rows * map->cols + 1)) {
    free(map->cells);
    return NULL;
  }

  return map;
}

/**
 * Sets up the maze structure from a file.
 * @param fptr Pointer to the file pointer for reading the maze.
 * @param map Pointer to the maze structure to be initialized.
 * @param file_name Name of the file containing the maze.
 * @return Initialized maze structure on success, or NULL on error.
 */
Map *init_maze(FILE **fptr, Map *map, char *file_name) {
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

  map->rows = size[0];
  map->cols = size[1];
  map->cells = ptr;

  for (int i = 0; i < size[0] * size[1]; i++) {
    map->cells[i] = 0;
  }

  return map;
}

/**
 * Validates a maze's structure from a file, checking its size and border rules.
 * @param file_name Path to the file containing the maze data.
 * @return Pointer to maze data if successful, NULL if any validation fails.
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

  // Check border validity
  // get neigbourts around one cell
  for (int row = 1; row <= maze.rows; row++) {
    for (int col = 1; col <= maze.cols; col++) {
      unsigned char neighbors[CELL_NEIGHBORS_COUNT];

      // For accessing maze.cells we need 0 indexing
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

      if (!valid_borders(maze.cells[cellIndex], neighbors)) {
        free(maze.cells);
        fclose(fptr);
        return NULL;
      }
    }
  }

  fclose(fptr);
  return maze.cells;
}

/**
 * Checks if the current cell's borders are compatible with its neighbors.
 * @param cell Value of the current cell
 * @param neighbors Array of neighbor cells' values in the order: [left, right,
 * up/down].
 * @return True if all neighboring sides are compatible with the current
 * cell, false otherwise.
 */
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
--lpath <R> <C> trues to find path in maze on entry on column R and row C. Path is searched by the rule of left hand\n\
--shortest <R> <C> tries to find shortest path in maze on entry on column R and row C.";

  printf("%s\n", help);
}
