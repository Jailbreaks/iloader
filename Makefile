CC = gcc
CFLAGS = -Wall -W -pedantic
CFLAGS += -Wno-long-long
CFLAGS += -O2
CFLAGS += -DUSE_SIGNAL=2
CFLAGS += -march=armv7 -mthumb

LD = gcc
LDFLAGS =
LDLIBS =

SOURCES = \
	lzss.c \
	iloader.c

OBJECTS = $(SOURCES:.c=.o)

.c.o:
	$(CC) -o $@ $(CFLAGS) -c $<

all: iloader

iloader: $(OBJECTS)
	$(LD) -o $@ $(LDFLAGS) $^ $(LDLIBS)

clean:
	-$(RM) $(OBJECTS)

realclean: clean
	-$(RM) iloader

dep:
	makedepend -Y $(SOURCES) -f depend

-include depend
