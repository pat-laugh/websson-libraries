# MIT License
# Copyright 2017 Patrick Laughrea

DIR_UTILS = ../utils
DIR_STRUCTURES = ../structures
DIR_PARSER = ../parser
DIR_SERIALIZER = ../serializer
DIR_SERIALIZER_HTML = ../serializerHtml
DIR_VARIOUS = ../various

OPTIONS = -std=c++11 -Wall -Wextra -Wno-missing-field-initializers -iquote ..
CPP_FILES = $(wildcard *.cpp)
OBJ_FILES = $(notdir $(CPP_FILES:.cpp=.o))
EXEC = tests.out

all: COMPILE_LIBS $(EXEC)

COMPILE_LIBS:
	cd $(DIR_UTILS); make -s
	cd $(DIR_STRUCTURES); make -s
	cd $(DIR_PARSER); make -s
	cd $(DIR_SERIALIZER); make -s
	cd $(DIR_SERIALIZER_HTML); make -s
	cd $(DIR_VARIOUS); make -s

$(EXEC): $(OBJ_FILES)
	$(CXX) -o $@ $^ -L$(DIR_PARSER) -lparser -lcurl \
	-L$(DIR_SERIALIZER) -lserializer -L$(DIR_SERIALIZER_HTML) -lserializerHtml \
	-L$(DIR_STRUCTURES) -lstructures -L$(DIR_UTILS) -lutils -L$(DIR_VARIOUS) -lvarious

%.o: %.cpp
	$(CXX) $(OPTIONS) -o $@ -c $<

clean:
	cd $(DIR_UTILS); make clean -s
	cd $(DIR_STRUCTURES); make clean -s
	cd $(DIR_PARSER); make clean -s
	cd $(DIR_SERIALIZER); make clean -s
	cd $(DIR_SERIALIZER_HTML); make clean -s
	cd $(DIR_VARIOUS); make clean -s
	rm -rf $(EXEC) *.o

debug: OPTIONS += -DNDEBUG -g3
debug: COMPILE_LIBS_DEBUG $(EXEC)

COMPILE_LIBS_DEBUG:
	cd $(DIR_UTILS); make debug -s
	cd $(DIR_STRUCTURES); make debug -s
	cd $(DIR_PARSER); make debug -s
	cd $(DIR_SERIALIZER); make debug -s
	cd $(DIR_SERIALIZER_HTML); make debug -s
	cd $(DIR_VARIOUS); make debug -s
