SHELL = /bin/sh

PREFIX = /usr/local
EPREFIX = $(PREFIX)
BINDIR = $(EPREFIX)/bin

LIBS = -lX11

.PHONY: all
all: tick

tick: tick.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< $(LIBS)

.PHONY: install
install:
	mkdir -p $(DESTDIR)$(BINDIR)
	cp -f tick $(DESTDIR)$(BINDIR)
	chmod 755 $(DESTDIR)$(BINDIR)/tick

.PHONY: uninstall
uninstall:
	rm -f $(DESTDIR)$(BINDIR)/tick

.PHONY: clean
clean:
	rm -f *.o tick
