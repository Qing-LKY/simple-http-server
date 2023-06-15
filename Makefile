export CC := gcc
export INCLUDE := include
export CFLAGS := -g -O2
export LDFLAGS := -lpthread
export TARGET := emu-shttpd

export SHELL := /bin/sh

all:
	$(MAKE) -C src
	$(MAKE) -C demo

.phony: clean install uninstall

clean:
	$(MAKE) -C src clean
	$(MAKE) -C demo clean
	$(RM) $(TARGET)