CC = gcc
CFLAGS = -c -Wall -O3 -std=gnu99
LDFLAGS = -lSDL2
SOURCES = chip8.c pwin.c test.c
OBJECTS = $(SOURCES:.c=.o)
EXECUTABLE = Vip8

all: $(SOURCES) $(EXECUTABLE)


$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

.PHONY: clean

clean:
	rm $(EXECUTABLE) $(OBJECTS)
