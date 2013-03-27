CC = cc
CFLAGS += -Wall
PREFIX ?= /usr/local
OBJS = main.o copyall.o copy.o screen.o log.o path.o misc.o color.o

all: $(OBJS)
	$(CC) $(CFLAGS) -o vcp $(OBJS) -lcurses

.c.o:
	$(CC) $(CFLAGS) -c -I/usr/include/ -o $@ $<
.SUFFIXS: .c .o

install:
	install -c -m 0555 -o root -g `./group.sh` vcp $(PREFIX)/bin

	if test -d $(PREFIX)/man/man1; then \
		install -c -m 0444 -o root -g `./group.sh` vcp.1.gz $(PREFIX)/man/man1; \
	fi;
	if test -d /usr/man/man1; then \
		install -c -m 0444 -o root -g `./group.sh` vcp.1.gz /usr/man/man1; \
	fi;
	if test -d /usr/share/man/man1; then \
		install -c -m 0444 -o root -g `./group.sh` vcp.1.gz /usr/share/man/man1; \
	fi;
	
	if test -d $(PREFIX)/man/pl/man1; then \
		install -c -m 0444 -o root -g `./group.sh` vcp.pl.1.gz $(PREFIX)/man/pl/man1/vcp.1.gz; \
	fi;
	if test -d /usr/man/pl/man1; then \
		install -c -m 0444 -o root -g `./group.sh` vcp.pl.1.gz /usr/man/pl/man1/vcp.1.gz; \
	fi;	
	if test -d /usr/share/man/pl/man1; then \
		install -c -m 0444 -o root -g `./group.sh` vcp.pl.1.gz /usr/share/man/pl/man1/vcp.1.gz; \
	fi;

installconf:
	install -c -m 0644 vcp.conf.sample /etc/vcp.conf
clean:
	-rm $(OBJS) vcp
