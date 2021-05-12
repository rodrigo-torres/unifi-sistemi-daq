CC = gcc
CFLAGS = -I.

DEPS = utility.h

TARGET = telnet_inspect

$(TARGET) : main.c $(DEPS)
	$(CC) -o $@ $< $(CFLAGS)

.PHONY: all

all: $(TARGET)

.PHONY: clean

clean:
	rm -f $(TARGET) *.o