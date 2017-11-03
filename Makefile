CC := g++

ifndef DEBUG
	DEBUG := 0
endif

override CXXFLAGS += -std=c++0x -DDEBUG=$(DEBUG)
override LDFLAGS += -lwiringPi -lmosquitto

OBJ_DIR := build
SRC_DIR := src
SRC := $(wildcard $(SRC_DIR)/temp/*.cpp)
TEMP_OBJ := $(patsubst %.cpp,%.o,$(subst $(SRC_DIR),$(OBJ_DIR),$(SRC)))
TEMP_OBJ_DIR := $(OBJ_DIR)/temp

.PHONY: print
print:
	echo $(SRC)
	echo $(TEMP_OBJ)

$(OBJ_DIR):
$(TEMP_OBJ_DIR): $(OBJ_DIR)
	@mkdir -p $@

$(TEMP_OBJ): $(OBJ_DIR)%.o: $(SRC_DIR)%.cpp | $(TEMP_OBJ_DIR)
	$(CC) $(CXXFLAGS) -o $@ -c $^

pwm: $(SRC_DIR)/pwm.c
	$(CC) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

temp: $(TEMP_OBJ)
	$(CC) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
