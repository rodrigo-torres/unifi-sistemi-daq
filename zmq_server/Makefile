CC = gcc
CFLAGS = -I. -lzmq

DEPS = utility.h

TARGET = zmq_server

$(TARGET) : main.c $(DEPS)
	$(CC) -o $@ $< $(CFLAGS)

.PHONY: all

all: $(TARGET)

.PHONY: clean

clean:
	rm -f $(TARGET) *.o
