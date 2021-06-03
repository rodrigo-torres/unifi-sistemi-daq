CC = gcc
CFLAGS = -I.

DEPS = gnuplot.h utility.h

TARGET = kdaq_client

$(TARGET) : main.c $(DEPS)
	$(CC) -o $@ $< $(CFLAGS)

.PHONY: all

all: $(TARGET)

.PHONY: clean

clean:
	rm -f $(TARGET) *.o
