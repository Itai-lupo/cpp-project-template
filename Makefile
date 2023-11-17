CC = clang
MAKEFLAGS += -j -w -s

MKDIR_P ?= mkdir -p


TARGET_EXEC ?= logger.out

BUILD_DIR ?= ./.build
OUTPUT_DIR ?= ./.output

INCLUDE_DIR ?= ./include/ $(BUILD_DIR)/include/
SRC_DIRS ?= ./src/ 
TESTS_DIR ?= ./tests

SCRIPTS_DIR ?= ./.scripts

PCH_PATH = ./include/core.hpp
PCH_OUT = $(BUILD_DIR)/include/core.hpp.gch

-include $(SCRIPTS_DIR)/Makefile.utils 
-include $(SCRIPTS_DIR)/Makefile.submodules

SRCS += $(foreach  dir,$(SRC_DIRS),$(call rwildcard,$(dir),*.c*)) 
OBJS :=  $(call turnInfoObjectFile,$(SRCS))  
DEPS := $(OBJS:.o=.d)
CPPFLAGS ?=  -std=c++20 -Wno-c99-designator
CFLAGS ?= -std=c99

INC_FLAGS := $(addprefix -I,$(INCLUDE_DIR))
CXXFLAGS += $(INC_FLAGS)  -MMD -MP -g -pthread -O0 -ggdb3 -Wall -Wextra -Werror -pedantic-errors 
LDFLAGS =  
LDLIBS +=  -lstdc++ -lm -lspdlog -lfmt

all:
	$(MAKE) $(BUILD_DIR)/include/files.json
	$(MAKE) $(PCH_OUT)
	compiledb  -o $(BUILD_DIR)/compile_commands.json $(MAKE) $(OUTPUT_DIR)/$(TARGET_EXEC)


$(OUTPUT_DIR)/$(TARGET_EXEC): $(BUILD_DIR)/include/files.json $(PCH_OUT) $(OBJS) 
	$(MKDIR_P) $(OUTPUT_DIR)
	$(CC)  $(OBJS) -o $@ $(LDFLAGS) $(LDLIBS)


run: all
	./$(OUTPUT_DIR)/$(TARGET_EXEC)


clean:
	$(RM) -r $(BUILD_DIR) 
	$(RM) -r $(OUTPUT_DIR)


-include $(SCRIPTS_DIR)/Makefile.tests
include $(SCRIPTS_DIR)/Makefile.doc	
-include $(SCRIPTS_DIR)/Makefile.build
-include $(SCRIPTS_DIR)/Makefile.pch
-include $(DEPS)


.PHONY: clean print all test run_test run build_test