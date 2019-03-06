#
# Makefile
#
# Originally written by Josh Vranish:
# https://spin.atomicobject.com/2016/08/26/makefile-c-projects/
#
# Moderately edited by John Alden
#

TARGET_EXEC_APP = main

MKDIR   := mkdir -p
RM      := rm -f
CP      := cp

ROOT_DIR := .

BLD_DIRS := $(ROOT_DIR)/out

SRC_DIRS := \
	$(ROOT_DIR)/src
INC_DIRS := \
	$(ROOT_DIR)/src \
	$(ROOT_DIR)/lib/nfd/src/include \
	$(ROOT_DIR)/lib/argparse
LIB_DIRS := \
	$(ROOT_DIR)/lib \
	$(ROOT_DIR)/lib/nfd/build/lib/Release/x64 \
	$(ROOT_DIR)/lib/argparse

LIBS := dl pthread SDL2 SDL2_ttf nfd z
NAMED_LIBS := libnfd.a libargparse.a
DEBUG_LIBS :=
DEBUG_FLAGS :=

ifeq ($(m), asan)
DEBUG_LIBS := asan
DEBUG_FLAGS := -fsanitize=address -fno-omit-frame-pointer
endif

ifeq ($(m), tsan)
DEBUG_LIBS := tsan
DEBUG_FLAGS := -fsanitize=thread
endif

ifeq ($(m), lsan)
DEBUG_LIBS := lsan
DEBUG_FLAGS := -fsanitize=leak
endif

ifeq ($(m), msan)
DEBUG_LIBS := msan
DEBUG_FLAGS := -fsanitize=memory
endif


SRCS := $(shell find $(SRC_DIRS) -name '*.cpp' -or -name '*.c')
OBJS := $(SRCS:%=$(BLD_DIRS)/%.o)
DEPS := $(OBJS:.o=.d)

INC_FLAGS       := $(addprefix -I,$(INC_DIRS))
LIB_FLAGS       := $(addprefix -L,$(LIB_DIRS))
LIB_STD_FLAGS   := $(addprefix -l,$(LIBS))
LIB_STD_FLAGS   += $(addprefix -l:,$(NAMED_LIBS))
DEBUG_LIB_FLAGS := $(addprefix -l,$(DEBUG_LIBS))

CFLAGS   += -MMD -MP -Wall $(shell pkg-config --cflags gtk+-3.0)
CXXFLAGS += -std=c++11
LDFLAGS  += $(shell pkg-config --libs gtk+-3.0)

.PHONY: all
all: LIB_FLAGS += $(LIB_STD_FLAGS)
all: libraries $(BLD_DIRS)/$(TARGET_EXEC_APP)

.PHONY: debug
debug: clean libraries
debug: CFLAGS += -ggdb $(DEBUG_FLAGS)
debug: LIB_FLAGS += $(DEBUG_LIB_FLAGS) $(LIB_STD_FLAGS)
debug: $(BLD_DIRS)/$(TARGET_EXEC_APP)

# Output App
$(BLD_DIRS)/$(TARGET_EXEC_APP): $(OBJS)
	$(CXX) $^ -o $@ $(LIB_FLAGS) $(LDFLAGS)

# c source
$(BLD_DIRS)/%.c.o: %.c
	$(MKDIR) $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@ $(INC_FLAGS)

# c++ source
$(BLD_DIRS)/%.cpp.o: %.cpp
	$(MKDIR) $(dir $@)
	$(CXX) $(CFLAGS) $(CXXFLAGS) -c $< -o $@ $(INC_FLAGS)

# Libraries Build Config
NFD_OS_NAME = windows
ifeq ($(OS),Windows_NT)
	NFD_OS_NAME := windows
else
	ifeq ($(shell uname),Linux)
		NFD_OS_NAME := linux
	endif
	ifeq ($(shell uname),Darwin)
		NFD_OS_NAME := macosx
	endif
endif

# Libraries Build
.PHONY: libraries
libraries:
	$(MAKE) -C lib/nfd/build/gmake_$(NFD_OS_NAME) nfd
	$(MAKE) -C lib/argparse libargparse.a

.PHONY: clean
clean:
	-$(RM) -r $(BLD_DIRS)

.PHONY: clobber
clobber: clean
	$(MAKE) -C lib/nfd/build/gmake_$(NFD_OS_NAME) clean
	$(MAKE) -C lib/argparse clean

# Miscellaneous
.PHONY: count
count:
	find $(SRC_DIRS) $(INC_DIRS) -name '*.cpp' -or -name '*.c' -or -name '*.hpp' -or -name '*.h' | grep -v "imgui\|GL\|KHR\|argparse\|nfd" | xargs wc -l

.PHONY: regen_config
regen_config:
	./misc/cfg_class_gen.py

-include $(DEPS)