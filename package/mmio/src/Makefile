#
# Copyright (C) 2012 Žilvinas Valinskas
# See LICENSE for more information.
#

CFLAGS	+= -Wall -Os

all:	mmio rtscts

libmmio.o: libmmio.c
	$(CC) $(CFLAGS) -c libmmio.c -o libmmio.o

utils.o: utils.c
	$(CC) $(CFLAGS) -c utils.c -o utils.o

mmio.o: mmio.c
	$(CC) $(CFLAGS) -c mmio.c -o mmio.o

strbuf.o : strbuf.c
	$(CC) $(CFLAGS) -c strbuf.c -o strbuf.o

mmio: mmio.o utils.o strbuf.o libmmio.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

rtscts: rtscts.o utils.o strbuf.o libmmio.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f *.o mmio rtscts
	rm -f tags cscope.*
