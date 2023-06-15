export CC := gcc
export INCLUDE := include
export CFLAGS := -g -O2
export LDFLAGS := -lpthread
export TARGET := emu-shttpd

export SHELL := /bin/sh

all:
	$(MAKE) -C src

.phony: clean install uninstall

clean:
	$(MAKE) -C src clean
	$(RM) $(TARGET)

install: all
	cp -f $(TARGET) /usr/local/bin
	cp -f example.conf /etc/emu-shttpd.conf

uninstall:
	$(RM) /usr/local/bin/$(TARGET)
	$(RM) /etc/emu-shttpd.conf