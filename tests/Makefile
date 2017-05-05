# MIT License
# Copyright(c) 2017 Patrick Laughrea

DIR_UTILS = ../utils
DIR_STRUCTURES = ../structures
DIR_PARSER = ../parser
DIR_SERIALIZER = ../serializer

CC = g++
OPTIONS = -std=c++11 -Wall -Wextra -Wno-missing-field-initializers -iquote ..
EXEC = tests.out

all: COMPILE_LIBS $(EXEC)

COMPILE_LIBS:
	cd $(DIR_UTILS); make -s
	cd $(DIR_STRUCTURES); make -s
	cd $(DIR_PARSER); make -s
	cd $(DIR_SERIALIZER); make -s

$(EXEC): tests.o
	$(CC) -o $@ -L$(DIR_UTILS) -lutils -L$(DIR_STRUCTURES) -lstructures -L$(DIR_PARSER) -lparser -L$(DIR_SERIALIZER) -lserializer $^ -lcurl

tests.o: tests.cpp
	$(CC) $(OPTIONS) -o $@ -c $<

clean:
	cd $(DIR_UTILS); make clean -s
	cd $(DIR_STRUCTURES); make clean -s
	cd $(DIR_PARSER); make clean -s
	cd $(DIR_SERIALIZER); make clean -s
	rm -rf $(EXEC) *.o

debug: OPTIONS += -DNDEBUG -g3
debug: COMPILE_LIBS_DEBUG $(EXEC)

COMPILE_LIBS_DEBUG:
	cd $(DIR_UTILS); make debug -s
	cd $(DIR_STRUCTURES); make debug -s
	cd $(DIR_PARSER); make debug -s
	cd $(DIR_SERIALIZER); make debug -s