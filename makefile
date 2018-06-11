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
	$(ROOT_DIR)/lib/nfd/src/include
LIB_DIRS := \
	$(ROOT_DIR)/lib \
	$(ROOT_DIR)/lib/nfd/build/lib/Release/x64

LIBS := dl SDL2 SDL2_ttf nfd

SRCS := $(shell find $(SRC_DIRS) -name '*.cpp' -or -name '*.c')
OBJS := $(SRCS:%=$(BLD_DIRS)/%.o)
DEPS := $(OBJS:.o=.d)

INC_FLAGS := $(addprefix -I,$(INC_DIRS))
LIB_FLAGS := $(addprefix -L,$(LIB_DIRS))
LIB_FLAGS += $(addprefix -l,$(LIBS))

CFLAGS   += -MMD -MP -Wall $(shell pkg-config --cflags gtk+-3.0)
CXXFLAGS += -std=c++11
LDFLAGS  += $(shell pkg-config --libs gtk+-3.0)

#package tool configs


.PHONY: all
all: libraries $(BLD_DIRS)/$(TARGET_EXEC_APP)

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

.PHONY: clean
clean:
	-$(RM) -r $(BLD_DIRS)

.PHONY: clobber
clobber: clean
	$(MAKE) -C lib/nfd/build/gmake_$(NFD_OS_NAME) clean

.PHONY: count
count:
	find $(SRC_DIRS) $(INC_DIRS) -name '*.cpp' -or -name '*.c' -or -name '*.hpp' -or -name '*.h' | xargs wc -l

-include $(DEPS)