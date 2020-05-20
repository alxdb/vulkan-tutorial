SRC := main.cc window.cc vulkan_app.cc vertex.cc
PKG := sdl2 vulkan
SHADERS := main.vert.glsl main.frag.glsl
BUILD_DIR := build
CXXFLAGS += --std=c++17 -I$(shell realpath .)

OBJ = $(SRC:.cc=.o)
DEP = $(SRC:.cc=.d)
SPIRV = $(SHADERS:.glsl=.spv.h)
LDLIBS += $(shell pkg-config --libs $(PKG))
CFLAGS += $(shell pkg-config --cflags $(PKG)) -g
CXXFLAGS += -MMD -MP $(CFLAGS)

LINK.o = $(LINK.cc)

vpath %.cc $(dir $(MAKEFILE_LIST))
vpath %.glsl $(dir $(MAKEFILE_LIST))

default:
	@mkdir -p build
	cd build && make -f ../Makefile all

all: shaders main

main: $(OBJ)

shaders: $(SPIRV)

%.spv.h: %.spv
	xxd -i $< > $@

%.spv: %.glsl
	glslc $< -o $@

.PHONY: clean

clean:
	rm -rf build

-include $(DEP)
