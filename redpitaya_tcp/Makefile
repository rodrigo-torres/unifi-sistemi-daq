CC = gcc
CFLAGS = -I.

DEPS = red_pitaya.h gnuplot.h utility.h

TARGET = redpitaya_tcp

$(TARGET) : main.c $(DEPS)
	$(CC) -o $@ $< $(CFLAGS)

.PHONY: all

all: $(TARGET)

.PHONY: clean

clean:
	rm -f $(TARGET) *.o