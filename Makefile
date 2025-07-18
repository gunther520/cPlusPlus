# Compiler and flags
CXX := g++
CXXFLAGS := -g -O0 -Wall -Wextra -std=c++17 -I./include -pthread
LDFLAGS := -L./lib/boost_1_88_0/stage/lib -lboost_thread -lboost_system -lboost_filesystem -pthread -Wl,-rpath=$(PWD)/lib/boost_1_88_0/stage/lib

# Directories
SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin
INCLUDE_DIR := include

# Source and object files
SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))

# Output binary
TARGET := $(BIN_DIR)/app

# Default target
all: $(TARGET)


# Link objects to create binary with rpath
$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -lboost_thread -lboost_system -lboost_filesystem -pthread -o $@

# Compile .cpp to .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build files
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

.PHONY: all clean
