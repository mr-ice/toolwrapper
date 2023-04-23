# Specify the C++ compiler and flags
CXX = g++
CXXFLAGS = -std=c++17

# Define the target executable name (default is "a.out")
TARGET = a.out

# Define the source file
SRC = wrapper.cpp

# Define the object file
OBJ = $(SRC:.cpp=.o)

# Define the default target (build the executable)
all: $(TARGET)

# Define the target to build the executable
$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Define the target to build the object file
$(OBJ): $(SRC)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Define the "clean" target to remove generated files
clean:
	rm -f $(OBJ) $(TARGET)
	rm -f bin/$(TARGET)

# Define the phony targets (targets that don't generate files)
.PHONY: all clean install

install: $(TARGET)
	cp $(TARGET) bin

rfp: read_file_find_path.o read_file_path.o
	$(CXX) $(CXXFLAGS) read_file_find_path.o read_file_path.o -o $@