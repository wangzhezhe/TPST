
CXX = g++ 

CXXFLAGS += -std=c++11 

all: groupmanager


groupmanager: 
	$(CXX) $(CXXFLAGS) $^ ./groupManager.cpp  -o $@

clean:
	find . -name "*.out" -delete
	find . -name "*.o" -delete
