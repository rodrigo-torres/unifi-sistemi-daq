
INCDIR = -I.
LIBS	 = -lgpib
DEPS   = common.h utility.h

CC = gcc
CFLAGS = -ggdb

TARGET = chat read write

chat : chat.c $(DEPS)
	$(CC) $(INCDIR) $(LIBS) $(CFLAGS) $< -o $@

read : read.c $(DEPS)
	$(CC) $(INCDIR) $(LIBS) $(CFLAGS) $< -o $@
	
write : write.c $(DEPS)
	$(CC) $(INCDIR) $(LIBS) $(CFLAGS) $< -o $@


.PHONY: all

all: $(TARGET)

.PHONY: clean

clean:
	rm -f $(TARGET)