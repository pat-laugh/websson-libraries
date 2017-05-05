# MIT License
# Copyright(c) 2017 Patrick Laughrea

CC = g++
OPTIONS = -std=c++11 -Wall -Wextra -Wno-missing-field-initializers
LIBRARY = libutils.a

all: $(LIBRARY)

$(LIBRARY): conType.o iterators.o utils.o utilsWebss.o
	ar -rc $(LIBRARY) $^

iterators.o: iterators.cpp
	$(CC) $(OPTIONS) -o $@ -c $<

conType.o: conType.cpp
	$(CC) $(OPTIONS) -o $@ -c $<

utils.o: utils.cpp
	$(CC) $(OPTIONS) -o $@ -c $<

utilsWebss.o: utilsWebss.cpp
	$(CC) $(OPTIONS) -o $@ -c $<

clean:
	rm -rf $(LIBRARY) *.o