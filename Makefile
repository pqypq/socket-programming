GXX = g++
OBJECTS = clientA clientB serverC serverT serverS serverP
FLAGS = -std=c++11
JUNKS = *.out $(OBJECTS)

all: $(OBJECTS)

serverC: central.cpp
	$(GXX) $(FLAGS) $^ -o $@

serverT: serverT.cpp edge.hpp graph.hpp
	$(GXX) $(FLAGS) $^ -o $@

serverP: serverP.cpp edge.hpp graph.hpp
	$(GXX) $(FLAGS) $^ -o $@

%: %.cpp
	$(GXX) $(FLAGS) $^ -o $@

clean:
	rm -rf $(JUNKS)
