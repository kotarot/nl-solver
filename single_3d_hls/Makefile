TARGET = sim
OBJS = $(CPPS:.cpp=.o)
CPPS = $(wildcard *.cpp)
CXX = g++
CXXFLAGS = -Wall -Wno-unknown-pragmas -DSOFTWARE

all: $(TARGET)

$(TARGET): $(OBJS)
#	$(CXX) -o $@ $(OBJS)  # <- for linux
	$(CXX) -Wl,--stack,4194304 -o $@ $(OBJS)  # <- for cygwin

clean:
	rm *.o
	rm $(TARGET)
