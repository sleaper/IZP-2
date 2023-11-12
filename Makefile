CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Werror

# Default target for development
dev: maze.c
	$(CC) $(CFLAGS) maze.c -o maze

# Production target
prod: maze.c
	$(CC) $(CFLAGS) -DNDEBUG maze.c -o maze

# You can keep your original target as well
maze: maze.c
	$(CC) $(CFLAGS) maze.c -o maze
