.PHONY: all clean test

all: mem_manager

CXXFLAGS = -g -std=c++11

mem_manager: memory_manager.o
	$(CXX) $(CXXFLAGS) -o $@ $<

memory_manager.o: memory_manager.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f *.o mem_manager


test: all
	./mem_manager
