CC := g++
CXXFLAGS := -std=c++0x -lwiringPi -lmosquitto

OBJ_DIR := build
SRC_DIR := src
SRC := $(wildcard $(SRC_DIR)/temp/*.cpp)
TEMP_OBJ := $(patsubst %.cpp,%.o,$(subst $(SRC_DIR),$(OBJ_DIR),$(SRC)))

.PHONY: print
print:
	echo $(SRC)
	echo $(TEMP_OBJ)

$(OBJ_DIR):
	@mkdir $(OBJ_DIR)

$(TEMP_OBJ): $(OBJ_DIR)%.o: $(SRC_DIR)%.cpp
	$(CC) $(CXXFLAGS) -c $^

pwm: $(SRC_DIR)/pwm.c
	$(CC) $(CXXFLAGS) -o $@ $^

temp: $(TEMP_OBJ)
	$(CC) $(CXXFLAGS) -o $@ $^
