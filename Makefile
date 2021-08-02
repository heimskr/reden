ifeq ($(BUILD),release)
BUILDFLAGS := -O3
else
BUILDFLAGS := -g -O0
endif

COMPILER   ?= clang++
CPPFLAGS   := -Wall -Wextra -std=c++20 $(BUILDFLAGS) -Ipingpong/include -Ipingpong/date/include
PPSRC      := $(shell find pingpong/src -name \*.cpp)
PPOBJ      := $(patsubst pingpong/src/%.cpp,pingpong/build/%.o,$(PPSRC))
SOURCES    := $(shell find src -name \*.cpp) src/resources.cpp
OBJECTS    := $(SOURCES:.cpp=.o)
DEPS       := gtk4 gtkmm-4.0 openssl libcurl
GTKFLAGS   := $(shell pkg-config --cflags $(DEPS))
LDFLAGS    := -L. -lpingpong -pthread $(shell pkg-config --libs $(DEPS))
OUTPUT     := reden
GLIB_COMPILE_RESOURCES = $(shell pkg-config --variable=glib_compile_resources gio-2.0)

.PHONY: all clean test

all: $(OUTPUT)

test: $(OUTPUT)
	./$(OUTPUT)

libpingpong.a: $(PPOBJ)
	@ printf "\e[2m[\e[22;35mar\e[39;2m]\e[22m $@\n"
	@ ar -rcs $@ $^

$(OUTPUT): libpingpong.a $(OBJECTS)
	@ printf "\e[2m[\e[22;36mld\e[39;2m]\e[22m $@\n"
	@ $(COMPILER) $(filter-out libpingpong.a,$^) -o $@ $(LDFLAGS)
ifeq ($(BUILD),release)
	strip $@
endif

%.o: %.cpp
	@ printf "\e[2m[\e[22;32mcc\e[39;2m]\e[22m $< \e[2m$(BUILDFLAGS)\e[22m\n"
	@ $(COMPILER) $(CPPFLAGS) $(GTKFLAGS) -Iinclude -c $< -o $@

pingpong/build/%.o: pingpong/src/%.cpp
	@ mkdir -p $(dir $@)
	@ printf "\e[2m[\e[22;32mcc\e[39;2m]\e[22m $< \e[2m$(BUILDFLAGS)\e[22m\n"
	@ $(COMPILER) $(CPPFLAGS) -c $< -o $@

src/resources.cpp: reden.gresource.xml $(shell $(GLIB_COMPILE_RESOURCES) --sourcedir=resources --generate-dependencies reden.gresource.xml)
	$(GLIB_COMPILE_RESOURCES) --target=$@ --sourcedir=resources --generate-source $<

clean:
	rm -rf $(OUTPUT) $(OBJECTS)

destroy:
	rm -rf $(OUTPUT) $(OBJECTS) $(PPOBJ) libpingpong.a

DEPFILE  := .dep
DEPTOKEN := "\# MAKEDEPENDS"

depend:
	@ echo $(DEPTOKEN) > $(DEPFILE)
	makedepend -f $(DEPFILE) -s $(DEPTOKEN) -- $(COMPILER) $(CPPFLAGS) -Iinclude -- $(SOURCES) 2>/dev/null
	@ rm $(DEPFILE).bak

sinclude $(DEPFILE)
