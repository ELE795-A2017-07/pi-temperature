CC := g++
USE_LORA := 1

ifndef DEBUG
	DEBUG := 0
endif

override CXXFLAGS += -std=c++0x -DDEBUG=$(DEBUG) -DUSE_LORA=$(USE_LORA)
override LDFLAGS += -lwiringPi -lmosquitto -lpthread

OBJ_DIR := build
SRC_DIR := src
SRC := $(wildcard $(SRC_DIR)/temp/*.cpp)
TEMP_OBJ := $(patsubst %.cpp,%.o,$(subst $(SRC_DIR),$(OBJ_DIR),$(SRC)))
TEMP_OBJ_DIR := $(OBJ_DIR)/temp
DEPS := $(patsubst %.cpp,%.d,$(subst $(SRC_DIR),$(OBJ_DIR),$(SRC)))

.PHONY: print
print:
	echo $(SRC)
	echo $(TEMP_OBJ)

$(OBJ_DIR):
$(TEMP_OBJ_DIR): $(OBJ_DIR)
	@mkdir -p $@

$(TEMP_OBJ): $(OBJ_DIR)%.o: $(SRC_DIR)%.cpp | $(TEMP_OBJ_DIR)
	$(CC) $(CXXFLAGS) -o $@ -c $<

pwm: $(SRC_DIR)/pwm.cpp
	$(CC) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

temp: $(TEMP_OBJ)
	$(CC) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

-include $(DEPS)
$(OBJ_DIR)/%.d: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -MF"$@" -MG -MM -MP -MT"$@" -MT"$(subst $(SRC_DIR),$(OBJ_DIR),$(<:.cpp=.o))" "$<"
