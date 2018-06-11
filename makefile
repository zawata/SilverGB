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

SRC_DIRS := $(ROOT_DIR)/src
BLD_DIRS := $(ROOT_DIR)/out
INC_DIRS := $(ROOT_DIR)/src
LIB_DIRS := $(ROOT_DIR)/lib

LIBS := dl SDL2 SDL2_ttf

SRCS := $(shell find $(SRC_DIRS) -name '*.cpp' -or -name '*.c')
OBJS := $(SRCS:%=$(BLD_DIRS)/%.o)
DEPS := $(OBJS:.o=.d)

INC_FLAGS := $(addprefix -I,$(INC_DIRS))
LIB_FLAGS := $(addprefix -L,$(LIB_DIRS))
LIB_FLAGS += $(addprefix -l,$(LIBS))

CFLAGS   += -MMD -MP -Wall
CXXFLAGS += -std=c++11
LDFLAGS  += 

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

.PHONY: count
count:
	find $(SRC_DIRS) $(INC_DIRS) -name '*.cpp' -or -name '*.c' -or -name '*.hpp' -or -name '*.h' | xargs wc -l

.PHONY: clean
clean:
	-$(RM) -r $(BLD_DIRS)

-include $(DEPS)