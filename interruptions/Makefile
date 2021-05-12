CC = gcc
CFLAGS = -ggdb -I.

DEPS   = utility.h
TARGET = interruptions

$(TARGET) : main.c $(DEPS)
	$(CC) -o $@ $< $(CFLAGS)

.PHONY: all

all: $(TARGET)

.PHONY: clean

clean:
	rm -f $(TARGET) *.o