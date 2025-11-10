SRC = server.c
OBJ = ${SRC:.c=.o}
CC = gcc
CFLAGS = -Wall -Wextra -Werror -Wshadow -Wno-missing-field-initializers -Wno-unused-parameter
LDFLAGS =
TARGET = server

all: server

TARGET: $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) -o $@

$(OBJ): config.h

config.h:
	cp config.def.h $@

.c.o:
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJ)

.PHONY: all clean

