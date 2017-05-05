# MIT License
# Copyright(c) 2017 Patrick Laughrea

OPTIONS = -std=c++11 -Wall -Wextra -Wno-missing-field-initializers
LIBRARY = libstructures.a

all: $(LIBRARY)

$(LIBRARY): block.o dictionary.o documentHead.o entity.o enum.o keywords.o list.o namespace.o paramBinary.o paramStandard.o templateBody.o tuple.o typeWebss.o webss.o
	ar -rc $(LIBRARY) $^

block.o: block.cpp
	$(CXX) $(OPTIONS) -o $@ -c $<

dictionary.o: dictionary.cpp
	$(CXX) $(OPTIONS) -o $@ -c $<

documentHead.o: documentHead.cpp
	$(CXX) $(OPTIONS) -o $@ -c $<

entity.o: entity.cpp
	$(CXX) $(OPTIONS) -o $@ -c $<
	
enum.o: enum.cpp
	$(CXX) $(OPTIONS) -o $@ -c $<
	
keywords.o: keywords.cpp
	$(CXX) $(OPTIONS) -o $@ -c $<
	
list.o: list.cpp
	$(CXX) $(OPTIONS) -o $@ -c $<
	
namespace.o: namespace.cpp
	$(CXX) $(OPTIONS) -o $@ -c $<

paramBinary.o: paramBinary.cpp
	$(CXX) $(OPTIONS) -o $@ -c $<

paramStandard.o: paramStandard.cpp
	$(CXX) $(OPTIONS) -o $@ -c $<

templateBody.o: templateBody.cpp
	$(CXX) $(OPTIONS) -o $@ -c $<

tuple.o: tuple.cpp
	$(CXX) $(OPTIONS) -o $@ -c $<

typeWebss.o: typeWebss.cpp
	$(CXX) $(OPTIONS) -o $@ -c $<

webss.o: webss.cpp
	$(CXX) $(OPTIONS) -o $@ -c $<

clean:
	rm -rf $(LIBRARY) *.o

debug: OPTIONS += -DNDEBUG -g3
debug: all