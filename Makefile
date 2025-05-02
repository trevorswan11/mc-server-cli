TARGET = servercli

CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c99

SRC = servercli.c
OBJ = $(SRC:.c=.o)

check-env:
	@test -f .env || (echo ".env file missing!" && false)

$(TARGET): check-env $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: clean

