maze: maze.c
	gcc -std=c11 -Wall -Wextra -Werror maze.c -o maze

test: maze
	@./test_maze.sh
