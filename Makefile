SRC := main.cc window.cc vulkan_app.cc
PKG := sdl2 vulkan
SHADERS := main.vert.glsl main.frag.glsl
BUILD_DIR := build
CXXFLAGS += --std=c++17

OBJ = $(SRC:.cc=.o)
DEP = $(SRC:.cc=.d)
SPIRV = $(SHADERS:.glsl=.spv)
LDLIBS += $(shell pkg-config --libs $(PKG))
CFLAGS += $(shell pkg-config --cflags $(PKG)) -g
CXXFLAGS += -MMD -MP $(CFLAGS)

LINK.o = $(LINK.cc)

vpath %.cc $(dir $(MAKEFILE_LIST))
vpath %.glsl $(dir $(MAKEFILE_LIST))

default:
	@mkdir -p build
	cd build && make -f ../Makefile all

all: main shaders

main: $(OBJ)

shaders: $(SPIRV)

%.spv: %.glsl
	glslc $^ -o $@

.PHONY: clean

clean:
	rm -rf build

-include $(DEP)
