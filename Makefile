# Compiler
CXX := g++

# Compiler flags (including OpenMP)
CXXFLAGS := -std=c++20 -fopenmp -O2

# Folder paths
SRC_DIR := src
BIN_DIR := bin

# Target executable
TARGET := $(BIN_DIR)/gpsa

# Source files and object files
SRCS := $(SRC_DIR)/main.cpp
OBJS := $(SRCS:$(SRC_DIR)/%.cpp=$(BIN_DIR)/%.o)

# Build target
all: $(TARGET)

# Link the object files to create the executable (with OpenMP support)
$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CXX) $(OBJS) -o $(TARGET) $(CXXFLAGS)

# Compile source files into object files (with OpenMP support)
$(BIN_DIR)/%.o: $(SRC_DIR)/%.cpp $(SRC_DIR)/helpers.hpp $(SRC_DIR)/implementation.hpp | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Ensure bin directory exists
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Clean up generated files
clean:
	rm -f $(OBJS) $(TARGET)

# Rebuild from scratch
rebuild: clean all

# Phony targets
.PHONY: all clean rebuild

