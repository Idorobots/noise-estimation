RM = rm -f
CC = gcc
#DEBUG = -DDEBUG
CFLAGS = $(DEBUG) -Isrc -Wall -Wextra -pedantic -std=c99
LDLIBS = -lopencv_core -lopencv_highgui -lopencv_imgproc

VPATH = src

OBJS = main.o config.o image.o utils.o homomorf.o

CONFIG = config.conf

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
	@ ./$(TARGET) $(CONFIG)
