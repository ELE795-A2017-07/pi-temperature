CC := g++
CXXFLAGS := -std=c++0x -lwiringPi -lmosquitto

pwm: pwm.o
	$(CC) $(CXXFLAGS) -o $@ $^

temp: temp.o
	$(CC) $(CXXFLAGS) -o $@ $^
