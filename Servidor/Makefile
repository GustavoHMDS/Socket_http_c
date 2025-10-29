SRC = servidor.c ../Utils/http.c
OUT = servidor

CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -g
LIBS = -lpthread

all:
	$(CC) $(SRC) -o $(OUT) $(CFLAGS) $(LIBS)

clean:
	rm -f $(OUT)
