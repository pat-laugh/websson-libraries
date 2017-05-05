# MIT License
# Copyright(c) 2017 Patrick Laughrea

OPTIONS = -std=c++11 -Wall -Wextra -Wno-missing-field-initializers
LIBRARY = libutils.a

all: $(LIBRARY)

$(LIBRARY): conType.o iterators.o utils.o utilsWebss.o
	ar -rc $(LIBRARY) $^

iterators.o: iterators.cpp
	$(CXX) $(OPTIONS) -o $@ -c $<

conType.o: conType.cpp
	$(CXX) $(OPTIONS) -o $@ -c $<

utils.o: utils.cpp
	$(CXX) $(OPTIONS) -o $@ -c $<

utilsWebss.o: utilsWebss.cpp
	$(CXX) $(OPTIONS) -o $@ -c $<

clean:
	rm -rf $(LIBRARY) *.o

debug: OPTIONS += -DNDEBUG -g3
debug: all