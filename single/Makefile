TARGET = solver 
OBJS = $(CPPS:.cpp=.o)
CPPS = $(wildcard *.cpp)
CXX = g++
CXXFLAGS = -Wall -O2 -pg

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) -pg -o $@ $(OBJS)

profiling.txt: gmon.out
	gprof $(TARGET) gmon.out > profiling.txt

clean:
	rm *.o
	rm $(TARGET)
