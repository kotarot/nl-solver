TARGET = viewer 
OBJS = $(CPPS:.cpp=.o)
CPPS = $(wildcard *.cpp)
CXX = g++
CXXFLAGS = -Wall -O2

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) -o $@ $(OBJS)

clean:
	rm *.o
	rm $(TARGET)
