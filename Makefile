CFLAGS+= -Wall
LDADD+= -lX11
LDFLAGS=
EXEC=bipolarbar

PREFIX?= /usr/local
BINDIR?= $(PREFIX)/bin

CC=gcc

all: $(EXEC)

bipolarbar: bipolarbar.o
	$(CC) $(LDFLAGS) -s -Os -o $@ $+ $(LDADD)

install: all
	install -Dm 755 bipolarbar $(DESTDIR)$(BINDIR)/bipolarbar

clean:
	rm -fv bipolarbar *.o

