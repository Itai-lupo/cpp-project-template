CC ?= clang
CXX ?= clang++
DB = gdb
MAKEFLAGS += -j 16 -s

TARGET_EXEC ?= logger.out

BUILD_DIR ?= $(PWD)/.build
OUTPUT_DIR ?= $(PWD)/.output

INCLUDE_DIR ?= $(PWD)/include/ $(BUILD_DIR)/include/
SRC_DIRS ?= ./src/ 
TESTS_DIR ?= ./tests

SCRIPTS_DIR ?= $(PWD)/.scripts

PCH_PATH = ./include/core.hpp
PCH_OUT = $(BUILD_DIR)/include/core.hpp.pch


DB_FLAGS ?= 

-include $(SCRIPTS_DIR)/Makefile.utils 
-include $(SCRIPTS_DIR)/Makefile.submodules

SRCS := $(foreach  dir,$(SRC_DIRS),$(call rwildcard,$(dir),*.c*)) 


OBJS :=  $(call turnInfoObjectFile,$(SRCS))  
DEPS := $(OBJS:.o=.d)

EXTRA_DEPENDENCIES += $(PCH_OUT)


INC_FLAGS := $(addprefix -I,$(INCLUDE_DIR))

CXXFLAGS += $(INC_FLAGS)  -MMD -MP -g -pthread -O2 -ggdb3 -Wall -Wextra -Werror -pedantic-errors 
CXXFLAGS +=  -Wno-gnu-zero-variadic-macro-arguments -Wno-gnu-anonymous-struct -Wno-nested-anon-types -Wno-c11-extensions 
CXXFLAGS += -fsized-deallocation -g3

CFLAGS ?= -std=c2x 
CPPFLAGS ?= -std=c++23 -Wno-c99-designator -Wno-c99-extensions -Wno-gnu-label-as-value 

LDFLAGS +=  
LDLIBS +=  -lstdc++ -lm -lfmt


-include $(SCRIPTS_DIR)/Makefile.fileId
-include $(SCRIPTS_DIR)/Makefile.tests
-include $(SCRIPTS_DIR)/Makefile.doc	
-include $(SCRIPTS_DIR)/Makefile.build
-include $(SCRIPTS_DIR)/Makefile.pch
-include $(DEPS)

all:
	$(MAKE) $(BUILD_DIR)/include/files.json
	compiledb --full-path --command-style  -o $(BUILD_DIR)/compile_commands.json $(MAKE) $(PCH_OUT)

	compiledb --full-path --command-style  -o $(BUILD_DIR)/compile_commands.json $(MAKE)  $(OUTPUT_DIR)/$(TARGET_EXEC)


$(OUTPUT_DIR)/$(TARGET_EXEC): $(EXTRA_DEPENDENCIES) $(OBJS) 
	$(MKDIR_P) $(OUTPUT_DIR)
	$(CC)  $(OBJS) -o $@ $(LDFLAGS) $(LDLIBS)


run: all
	GLIBC_TUNABLES=glibc.pthread.rseq=0 $(OUTPUT_DIR)/$(TARGET_EXEC)

debug: all
	GLIBC_TUNABLES=glibc.pthread.rseq=0 $(OUTPUT_DIR)/$(TARGET_EXEC) & sudo -E nsenter -e -a -t `pgrep logger.out | sed -n '2p'` $(DB) -p 1 $(DB_FLAGS) -ex "break"



clean:
	$(RM) -r $(BUILD_DIR)/* 
	$(RM) -r $(OUTPUT_DIR)/*

.PHONY: clean print all test run_test run build_test debug_test debug watch utils

