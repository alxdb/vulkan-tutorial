SRC = main.cc window.cc
OBJ = $(subst .cc,.o,$(SRC))

PKG=sdl2 vulkan
LDLIBS += $(shell pkg-config --libs $(PKG))
CFLAGS += $(shell pkg-config --cflags $(PKG)) -g
CXXFLAGS += $(CFLAGS) -Wall --std=c++17

LINK.o = $(LINK.cc)

all: main tags

main: $(OBJ)

window.o: window.hh

main.o: window.o

.PHONY: clean tags

clean:
	rm -f $(OBJ)
	rm -f main

tags:
	ctags -R .
