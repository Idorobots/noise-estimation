RM = rm -f
CC = gcc
CFLAGS = -Isrc -Wall -Wextra -pedantic -std=c99
LDLIBS = 

VPATH = src

OBJS = main.o config.o

TARGET = ne

all: $(TARGET)

$(TARGET): $(OBJS)
	 $(CC) $^ $(LDLIBS) -o$@

%.o: %.c
	$(CC) $(CFLAGS) $^ -c

.PHONY: clean run

clean:
	$(RM) $(TARGET)
	$(RM) $(OBJS)

run:
	@ ./$(TARGET)
