SHELL = /bin/sh
UNAME = $(shell uname -s | tr '[A-Z]' '[a-z]')

ifeq ($(UNAME), darwin)
# Mac OS X
CC ?= clang
DLLFLAGS ?= -bundle -undefined dynamic_lookup
else
# assume Linux
CC ?= gcc
DLLFLAGS ?= -shared
endif

RM ?= rm -f

V ?= 5.2
LUADIR ?= /usr/local/include/lua$V

COPT ?= -O2
CVERSION ?= -ansi -pedantic
CWARN ?= -Wall -Wextra
CFLAGS = $(COPT) $(CVERSION) $(CWARN) -I$(LUADIR) -fPIC $(MYFLAGS)

all: capi.so

clean:
	$(RM) *.o *.so

%.so: %.o $(MYOBJ)
	$(CC) $(DLLFLAGS) -o $@ $^

%.o: %.c $(MYSRC)
	$(CC) $(CFLAGS) -c -o $@ $^
