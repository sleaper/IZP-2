# Maze Solver
This program navigates through a maze defined in a text file using a rectangular matrix of integers. The objective is to print the coordinates of the path from the maze's entrance to its exit.

## Compilation and Execution
Compile the program using:
```bash
$ gcc -std=c11 -Wall -Wextra -Werror maze.c -o maze
```
Run the program in the following ways:
```bash
./maze --help
./maze --test file.txt
./maze --rpath R C file.txt
./maze --lpath R C file.txt
./maze --shortest R C file.txt (Optional Premium Feature)
```
```bash
$ ./maze --shortest 3 7 bludiste.txt
3,7
2,7
2,6
2,5
2,4
1,4
1,3
1,2
1,1
```
## Features
- Maze Validation: Checks if the provided maze file is correctly formatted.
- Pathfinding: Supports different algorithms to find a path using the right-hand or left-hand rules, or the shortest path.
- Output: Prints a sequence of coordinates representing the path through the maze.

![image](https://github.com/sleaper/IZP-1/assets/19505081/29ef62a0-9081-481b-af12-2bfa37aebb6e)

## File Format
The maze is represented in a text file as a series of numbers:
- The first two numbers define the dimensions of the maze.
- Each subsequent line contains values for each cell, where a 3-bit value defines the boundaries of each triangular cell.

Example:
```
6 7
1 4 4 2 5 0 6
1 4 4 0 4 0 2
1 0 4 0 4 6 1
1 2 7 1 0 4 2
3 1 4 2 3 1 2
4 2 5 0 4 2 5
```

### Example Input and Output
Mazes are solved by providing starting coordinates, and the program outputs the sequence of moves to navigate through the maze.
