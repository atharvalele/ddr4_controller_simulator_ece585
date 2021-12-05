###############################################################
# ECE585 - Microprocessor System Design
# Memory Controller Simulator
#
# Group 9: Atharva Lele, Ayush Srivastava
#		   Supreet Gulavani, Yashodhan Wagle
###############################################################

# Target
TARGET		:= ece585_memory_controller

# Directories for source files and builds
SRC_DIR 	:= src
BUILD_DIR 	:= build
OBJ_DIR		:= $(BUILD_DIR)/obj
INCLUDE 	:= -Iinclude/

# Sources and objects
SRCS	:= $(wildcard $(SRC_DIR)/*.cpp)
OBJS	:= $(SRCS:%.cpp=$(OBJ_DIR)/%.o)
DEPS	:= $(OBJS:.o=.d)

# Compiler
CXX			= clang++
CXXFLAGS 	= -Wall -Werror -std=c++11

# Build Recipies
all: build $(TARGET)

# Build all the object files
$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

$(TARGET): $(OBJS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $^

build:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(OBJ_DIR)

debug: CXXFLAGS += -g -DDEBUG
debug: all

release: all

.PHONY: all clean debug release build info

clean:
	rm -rf $(BUILD_DIR) $(TARGET)
	@echo "Cleanup done!"

info:
	@echo "Application:\t" $(TARGET)
	@echo "Object Dir:\t" $(OBJ_DIR)
	@echo "Sources:\t" $(SRCS)
	@echo "Objects:\t" $(OBJS)
	@echo "Dependencies:\t" $(DEPS)