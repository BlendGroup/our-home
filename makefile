SRC_DIR := src
TEST_DIR := test
LIB_DIR := lib
OBJ_DIR := build
SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)
TEST_FILES := $(wildcard $(TEST_DIR)/*.cpp)
LIB_FILES := $(wildcard $(LIB_DIR)/*.cpp)
OBJ_FILES_RUN := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC_FILES)) $(patsubst $(LIB_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(LIB_FILES))
OBJ_FILES_TEST := $(patsubst $(TEST_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(TEST_FILES)) $(patsubst $(LIB_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(LIB_FILES))
LIB_DEPS := -lX11 -lGL -lGLEW -lassimp -lOpenCL -lopenal -lalut
LD_FLAGS := -L ./lib
export LD_LIBRARY_PATH := :./lib

all: run

run: build main.run
	./main.run

test: build main.test
	./main.test

build:
	mkdir build

main.test: $(OBJ_FILES_TEST)
	g++ $^ $(LD_FLAGS) -o $@ $(LIB_DEPS)

main.run: $(OBJ_FILES_RUN)
	g++ $^ $(LD_FLAGS) -o $@ $(LIB_DEPS)

$(OBJ_DIR)/%.o: $(LIB_DIR)/%.cpp
	g++ -c -o $@ $< -I ./include -D DEBUG

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	g++ -c -o $@ $< -I ./include -D DEBUG

$(OBJ_DIR)/%.o: $(TEST_DIR)/%.cpp
	g++ -c -o $@ $< -I ./include -D DEBUG

clean:
	rm -f $(OBJ_FILES_RUN)
	rm -f $(OBJ_FILES_TEST)
	rm -f main.run