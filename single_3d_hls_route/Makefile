TARGET = sim
OBJS = $(CPPS:.cpp=.o)
CPPS = $(wildcard *.cpp)
CXX = g++
CXXFLAGS = -Wall -Wno-unknown-pragmas -DSOFTWARE

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) -o $@ $(OBJS)

clean:
	rm *.o
	rm $(TARGET)
