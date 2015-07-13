TARGET = solver 
OBJS = $(CPPS:.cpp=.o)
CPPS = $(wildcard *.cpp)
CXX = g++

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) -o $@ $(OBJS)

clean:
	rm *.o
	rm $(TARGET)
