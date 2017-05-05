# MIT License
# Copyright(c) 2017 Patrick Laughrea

CC = g++
OPTIONS = -std=c++11 -Wall -Wextra -Wno-missing-field-initializers
LIBRARY = libstructures.a

all: $(LIBRARY)

$(LIBRARY): block.o dictionary.o documentHead.o entity.o enum.o keywords.o list.o namespace.o paramBinary.o paramStandard.o templateBody.o tuple.o typeWebss.o webss.o
	ar -rc $(LIBRARY) $^

block.o: block.cpp
	$(CC) $(OPTIONS) -o $@ -c $<

dictionary.o: dictionary.cpp
	$(CC) $(OPTIONS) -o $@ -c $<

documentHead.o: documentHead.cpp
	$(CC) $(OPTIONS) -o $@ -c $<

entity.o: entity.cpp
	$(CC) $(OPTIONS) -o $@ -c $<
	
enum.o: enum.cpp
	$(CC) $(OPTIONS) -o $@ -c $<
	
keywords.o: keywords.cpp
	$(CC) $(OPTIONS) -o $@ -c $<
	
list.o: list.cpp
	$(CC) $(OPTIONS) -o $@ -c $<
	
namespace.o: namespace.cpp
	$(CC) $(OPTIONS) -o $@ -c $<

paramBinary.o: paramBinary.cpp
	$(CC) $(OPTIONS) -o $@ -c $<

paramStandard.o: paramStandard.cpp
	$(CC) $(OPTIONS) -o $@ -c $<

templateBody.o: templateBody.cpp
	$(CC) $(OPTIONS) -o $@ -c $<

tuple.o: tuple.cpp
	$(CC) $(OPTIONS) -o $@ -c $<

typeWebss.o: typeWebss.cpp
	$(CC) $(OPTIONS) -o $@ -c $<

webss.o: webss.cpp
	$(CC) $(OPTIONS) -o $@ -c $<

clean:
	rm -rf $(LIBRARY) *.o

debug: OPTIONS += -DNDEBUG -g3
debug: all