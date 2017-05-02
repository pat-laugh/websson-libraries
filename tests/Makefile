# MIT License
# Copyright(c) 2017 Patrick Laughrea

DIR_UTILS = ../utils
DIR_STRUCTURES = ../structures
DIR_PARSER = ../parser
DIR_SERIALIZER = ../serializer

CC = g++
OPTIONS = -std=c++11 -Wall -iquote ..
EXEC = tests.out

all: $(EXEC)

$(EXEC): tests.o
	$(CC) -o $@ -L$(DIR_UTILS) -lutils -L$(DIR_STRUCTURES) -lstructures -L$(DIR_PARSER) -lparser -L$(DIR_SERIALIZER) -lserializer $^

tests.o: tests.cpp
	$(CC) $(OPTIONS) -o $@ -c $<

clean:
	rm -rf $(EXEC) *.o