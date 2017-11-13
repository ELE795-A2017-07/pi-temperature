PRGM := lora_pingpong
CXX := g++
#CXXFLAGS := -Wall -Wextra
LDFLAGS := -lpthread
SRC := $(wildcard *.cpp)
OBJ := $(patsubst %.cpp,%.o,$(SRC))

$(PRGM): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

.PHONY: run
run: $(PRGM)
	sudo ./$(PRGM)
