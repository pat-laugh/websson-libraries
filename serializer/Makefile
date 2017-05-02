# MIT License
# Copyright(c) 2017 Patrick Laughrea

DIR_UTILS = ../utils
DIR_STRUCTURES = ../structures

CC = g++
OPTIONS = -std=c++11 -Wall -iquote .. -L$(DIR_UTILS) -lutils -L$(DIR_STRUCTURES) -lstructures
LIBRARY = libserializer.a

all: $(LIBRARY)

$(LIBRARY): serializer.o serializerBinary.o
	ar -rc $(LIBRARY) $^

serializer.o: serializer.cpp
	$(CC) $(OPTIONS) -o $@ -c $<

serializerBinary.o: serializerBinary.cpp
	$(CC) $(OPTIONS) -o $@ -c $<

clean:
	rm -rf $(LIBRARY) *.o