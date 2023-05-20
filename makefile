SRC_DIR := src
LIB_DIR := lib
OBJ_DIR := build
SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)
LIB_FILES := $(wildcard $(LIB_DIR)/*.cpp)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC_FILES)) $(patsubst $(LIB_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(LIB_FILES))
LIB_DEPS := -lX11 -lGL -lGLEW -lassimp -lOpenCL
LD_FLAGS := -L ./lib

all: run

run: main.run

# test: main.test

# main.test:

main.run: $(OBJ_FILES)
	# echo $^
	g++ $^ $(LD_FLAGS) -o $@ $(LIB_DEPS)

$(OBJ_DIR)/%.o: ($(SRC_DIR)/%.cpp) ($(LIB_DIR)/%.cpp)
	g++ -c -o $@ $< -I ./include 

clean:
	rm -f $(OBJ_FILES)
	rm -f main.run