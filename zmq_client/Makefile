CC = gcc
#CFLAGS = -DTEST_CLIENT -I. -lzmq
CFLAGS = -I. -lzmq

DEPS = gnuplot.h utility.h

TARGET = zmq_client 

$(TARGET) : main.c $(DEPS)
	$(CC) -o $@ $< $(CFLAGS)

.PHONY: all

all: $(TARGET)

.PHONY: clean

clean:
	rm -f $(TARGET) *.o
