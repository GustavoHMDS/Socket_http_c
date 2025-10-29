SRC = cliente.c ../Utils/http.c
OUT = cliente

CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -g

all:
	$(CC) $(SRC) -o $(OUT) $(CFLAGS)

clean:
	rm -f $(OUT)