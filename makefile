CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic

SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

VERSION = 0.2

SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))

PARSER_OBJS = $(OBJ_DIR)/load_config.o $(OBJ_DIR)/main.o
WRAPPER_OBJS = $(OBJ_DIR)/load_config.o $(OBJ_DIR)/wrapper.o

TARGET = $(BIN_DIR)/config_parser

all: $(TARGET) $(BIN_DIR)/wrapper-v$(VERSION)

$(BIN_DIR)/wrapper-v$(VERSION): $(WRAPPER_OBJS)
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(TARGET): $(PARSER_OBJS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

.PHONY: all clean
