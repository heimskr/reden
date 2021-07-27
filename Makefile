CC         ?= g++
BUILDFLAGS ?= -O3
CFLAGS     := -Wall -Wextra -std=c++20 $(BUILDFLAGS) -Ipingpong/include
PPSRC      := $(shell find pingpong/src -name \*.cpp)
PPOBJ      := $(patsubst pingpong/src/%.cpp,pingpong/build/%.o,$(PPSRC))
OUTPUT     := reden

.PHONY: all

all: $(PPOBJ)

pingpong/build/%.o: pingpong/src/%.cpp
	@ mkdir -p $(dir $@)
	@ printf "\e[2m[\e[22;32mCC\e[39;2m]\e[22m $< \e[2m$(BUILDFLAGS)\e[22m\n"
	@ $(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf pingpong/build $(OUTPUT)
