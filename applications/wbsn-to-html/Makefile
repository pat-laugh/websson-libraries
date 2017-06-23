# MIT License
# Copyright 2017 Patrick Laughrea

DIR_UTILS = ../../utils
DIR_STRUCTURES = ../../structures
DIR_PARSER = ../../parser
DIR_SERIALIZER_HTML = ../../serializerHtml

OPTIONS = -std=c++11 -Wall -Wextra -Wno-missing-field-initializers -iquote ../..
CPP_FILES = $(wildcard *.cpp)
OBJ_FILES = $(notdir $(CPP_FILES:.cpp=.o))
EXEC = wbsn-to-html.out

all: COMPILE_LIBS $(EXEC)

COMPILE_LIBS:
	cd $(DIR_UTILS); make -s
	cd $(DIR_STRUCTURES); make -s
	cd $(DIR_PARSER); make -s
	cd $(DIR_SERIALIZER_HTML); make -s

$(EXEC): $(OBJ_FILES)
	$(CXX) -o $@ $^ -L$(DIR_PARSER) -lparser -lcurl \
	-L$(DIR_SERIALIZER_HTML) -lserializerHtml \
	-L$(DIR_STRUCTURES) -lstructures -L$(DIR_UTILS) -lutils

%.o: %.cpp
	$(CXX) $(OPTIONS) -o $@ -c $<

clean:
	cd $(DIR_UTILS); make clean -s
	cd $(DIR_STRUCTURES); make clean -s
	cd $(DIR_PARSER); make clean -s
	cd $(DIR_SERIALIZER_HTML); make clean -s
	rm -rf $(EXEC) *.o

debug: OPTIONS += -DNDEBUG -g3
debug: COMPILE_LIBS_DEBUG $(EXEC)

COMPILE_LIBS_DEBUG:
	cd $(DIR_UTILS); make debug -s
	cd $(DIR_STRUCTURES); make debug -s
	cd $(DIR_PARSER); make debug -s
	cd $(DIR_SERIALIZER_HTML); make debug -s
