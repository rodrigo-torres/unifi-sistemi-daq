
INCDIR = -I.
LIBS	 = -lgpib
DEPS   = common.h gpib_daq.h gnuplot.h utility.h

CC = gcc
CFLAGS = -ggdb

TARGET = gpib_daq

$(TARGET) : chat.c $(DEPS)
	$(CC) $(INCDIR) $(LIBS) $(CFLAGS) $< -o $@

.PHONY: all

all: $(TARGET)

.PHONY: clean

clean:
	rm -f $(TARGET)