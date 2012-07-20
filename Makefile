CFLAGS+= -Wall
LDADD+= -lX11
LDFLAGS=
EXEC=splitter_bar

PREFIX?= /usr/local
BINDIR?= $(PREFIX)/bin

CC=gcc

all: $(EXEC)

splitter_bar: splitter_bar.o
	$(CC) $(LDFLAGS) -s -O2 -o $@ $+ $(LDADD)

install: all
	install -Dm 755 splitter_bar $(DESTDIR)$(BINDIR)/splitter_bar

clean:
	rm -fv splitter_bar *.o

