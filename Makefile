CC = gcc

nosh: src/*.c
	@${CC} src/*.c -o nosh -Wall -Wextra -I src/include -s
