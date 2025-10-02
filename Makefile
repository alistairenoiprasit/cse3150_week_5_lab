# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -g -Wall -Wextra

# Target executable
TARGET = solution

# Source files
SOURCES = solution.cpp

# Object files
OBJECTS = $(SOURCES:.cpp=.o)

# Default target
all: $(TARGET)

# Build the executable
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJECTS)

# Compile source files to object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Run tests using pytest
test: $(TARGET)
	pytest test_game.py

# Clean up compiled files
clean:
	rm -f $(TARGET) $(OBJECTS)

# Phony targets (not actual files)
.PHONY: all test clean