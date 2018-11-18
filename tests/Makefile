# MIT License
# Copyright 2017-2018 Patrick Laughrea

DIR_PROJECT = ..
DIR_UTILS = $(DIR_PROJECT)/utils
DIR_STRUCTURES = $(DIR_PROJECT)/structures
DIR_PARSER = $(DIR_PROJECT)/parser
DIR_SERIALIZER = $(DIR_PROJECT)/serializer
DIR_SERIALIZER_HTML = $(DIR_PROJECT)/serializerHtml
DIR_VARIOUS = $(DIR_PROJECT)/various

IS_MAC = $$(test "$$(uname -s)" = 'Darwin' && echo 1 || echo 0)

OPTIONS = -std=c++11 -Wall -Wextra -Wno-missing-field-initializers -I $(DIR_PROJECT)
CPP_FILES = $(wildcard *.cpp)
OBJ_FILES = $(notdir $(CPP_FILES:.cpp=.o))
EXEC = tests.out

all: compile_libs $(EXEC)

disable_import: DISABLE_IMPORT = 1
disable_import: compile_libs $(EXEC)

compile_libs:
	cd $(DIR_UTILS); make -s
	cd $(DIR_STRUCTURES); make -s
	cd $(DIR_PARSER); \
	if [ -n "$(DISABLE_IMPORT)" ]; then \
		make disable_import -s; \
	else \
		make -s; \
	fi
	cd $(DIR_SERIALIZER); make -s
	cd $(DIR_SERIALIZER_HTML); make -s
	cd $(DIR_VARIOUS); make -s

$(EXEC): $(OBJ_FILES)
	$(CXX) -o $@ $^ -L$(DIR_PARSER) -lparser \
	$$( \
		if [ -n "$(DISABLE_IMPORT)" ]; then \
			echo ''; \
		else \
			echo ' -lcurl'; \
			if [ $(IS_MAC) -eq 1 ]; then \
				echo ' -L/opt/local/lib -lboost_filesystem-mt -lboost_system-mt'; \
			else \
				echo ' -lboost_filesystem -lboost_system'; \
			fi \
		fi \
	) \
	-L$(DIR_SERIALIZER) -lserializer -L$(DIR_SERIALIZER_HTML) -lserializerHtml \
	-L$(DIR_STRUCTURES) -lstructures -L$(DIR_UTILS) -lutils -L$(DIR_VARIOUS) -lvarious
	test $(IS_MAC) -eq 1 && \
			for s in parser serializer serializerHtml structures utils various; do \
				install_name_tool -change lib$$s.so $$(pwd)/../$$s/lib$$s.so $(EXEC); \
			done

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
debug: compile_libs_debug $(EXEC)

compile_libs_debug:
	cd $(DIR_UTILS); make debug -s
	cd $(DIR_STRUCTURES); make debug -s
	cd $(DIR_PARSER); make debug -s
	cd $(DIR_SERIALIZER); make debug -s
	cd $(DIR_SERIALIZER_HTML); make debug -s
	cd $(DIR_VARIOUS); make debug -s
