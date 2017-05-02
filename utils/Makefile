# MIT License
# Copyright(c) 2017 Patrick Laughrea

CC = g++
OPTIONS = -std=c++11 -Wall
LIBRARY = libutils.a

all : $(LIBRARY)

$(LIBRARY): conType.o utils.o utilsWebss.o
	ar -rc $(LIBRARY) $^

conType.o: conType.cpp
	$(CC) $(OPTIONS) -o $@ -c $<

utils.o: utils.cpp
	$(CC) $(OPTIONS) -o $@ -c $<

utilsWebss.o: utilsWebss.cpp
	$(CC) $(OPTIONS) -o $@ -c $<

clean:
	rm $(LIBRARY) *.o		