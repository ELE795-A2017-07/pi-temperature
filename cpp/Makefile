CC := g++
LIB_SRC := $(wildcard libs/*.cpp)
LIB_OBJ := $(patsubst %.cpp,%.o,$(LIB_SRC))

pwm: pwm.o $(LIB_OBJ)
	$(CC) $(CXXFLAGS) -o $@ $^
