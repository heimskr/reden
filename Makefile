COMPILER   ?= g++
BUILDFLAGS ?= -O3
CFLAGS     := -Wall -Wextra -std=c++20 $(BUILDFLAGS) -Ipingpong/include
PPSRC      := $(shell find pingpong/src -name \*.cpp)
PPOBJ      := $(patsubst pingpong/src/%.cpp,pingpong/build/%.o,$(PPSRC))
SOURCES    := $(shell find src -name \*.cpp) src/resources.cpp
OBJECTS    := $(SOURCES:.cpp=.o)
DEPS       := gtk4 gtkmm-4.0
GTKFLAGS   := $(shell pkg-config --cflags $(DEPS))
LDFLAGS    := $(shell pkg-config --libs $(DEPS)) -L. -lpingpong -pthread
OUTPUT     := reden
GLIB_COMPILE_RESOURCES = $(shell pkg-config --variable=glib_compile_resources gio-2.0)

.PHONY: all clean test

all: $(OUTPUT)

test: $(OUTPUT)
	./$(OUTPUT)

libpingpong.a: $(PPOBJ)
	ar -rcs $@ $^

$(OUTPUT): libpingpong.a $(OBJECTS)
	@ printf "\e[2m[\e[22;36mLD\e[39;2m]\e[22m $@\n"
	@ $(COMPILER) $(filter-out libpingpong.a,$^) -o $@ $(LDFLAGS)

%.o: %.cpp
	@ printf "\e[2m[\e[22;32mCC\e[39;2m]\e[22m $< \e[2m$(BUILDFLAGS)\e[22m\n"
	@ $(COMPILER) $(CFLAGS) $(GTKFLAGS) -Iinclude -c $< -o $@

pingpong/build/%.o: pingpong/src/%.cpp
	@ mkdir -p $(dir $@)
	@ printf "\e[2m[\e[22;32mCC\e[39;2m]\e[22m $< \e[2m$(BUILDFLAGS)\e[22m\n"
	@ $(COMPILER) $(CFLAGS) -c $< -o $@

src/resources.cpp: reden.gresource.xml $(shell $(GLIB_COMPILE_RESOURCES) --sourcedir=resources --generate-dependencies reden.gresource.xml)
	$(GLIB_COMPILE_RESOURCES) --target=$@ --sourcedir=resources --generate-source $<

clean:
	rm -rf $(OUTPUT) $(OBJECTS)

destroy:
	rm -rf $(OUTPUT) $(OBJECTS) $(PPOBJ) libpingpong.a
