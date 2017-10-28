# MIT License
# Copyright 2017 Patrick Laughrea

DIR_PROJECT = ../..
DIR_UTILS = $(DIR_PROJECT)/utils
DIR_STRUCTURES = $(DIR_PROJECT)/structures
DIR_PARSER = $(DIR_PROJECT)/parser
DIR_SERIALIZER_HTML = $(DIR_PROJECT)/serializerHtml
DIR_VARIOUS = $(DIR_PROJECT)/various

OPTIONS = -std=c++11 -Wall -Wextra -Wno-missing-field-initializers -I $(DIR_PROJECT)
CPP_FILES = $(wildcard *.cpp)
OBJ_FILES = $(notdir $(CPP_FILES:.cpp=.o))
EXEC = wbsn-to-html.out

all: compile_libs $(EXEC)

disable_import: DISABLE_IMPORT = 1
disable_import: compile_libs $(EXEC)

compile_libs:
	cd $(DIR_UTILS); make -s
	cd $(DIR_STRUCTURES); make -s
	cd $(DIR_PARSER); \
	if [ $(DISABLE_IMPORT) ]; then \
		make disable_import -s; \
	else \
		make -s; \
	fi
	cd $(DIR_SERIALIZER_HTML); make -s
	cd $(DIR_VARIOUS); make -s

$(EXEC): $(OBJ_FILES)
	$(CXX) -o $@ $^ -L$(DIR_PARSER) -lparser \
	$$( \
		if [ $(DISABLE_IMPORT) ]; then \
			echo ''; \
		else \
			echo ' -lcurl'; \
			if [ "$$(uname -s)" = "Darwin" ]; then \
				echo '# Mac OS' > /dev/null; \
				echo ' -L/opt/local/lib -lboost_filesystem-mt -lboost_system-mt'; \
			else \
				echo '# Linux' > /dev/null; \
				echo ' -lboost_filesystem -lboost_system'; \
			fi \
		fi \
	) \
	-L$(DIR_SERIALIZER_HTML) -lserializerHtml \
	-L$(DIR_STRUCTURES) -lstructures -L$(DIR_UTILS) -lutils -L$(DIR_VARIOUS) -lvarious

%.o: %.cpp
	$(CXX) $(OPTIONS) -o $@ -c $<

clean:
	cd $(DIR_UTILS); make clean -s
	cd $(DIR_STRUCTURES); make clean -s
	cd $(DIR_PARSER); make clean -s
	cd $(DIR_SERIALIZER_HTML); make clean -s
	cd $(DIR_VARIOUS); make clean -s
	rm -rf $(EXEC) *.o

debug: OPTIONS += -DNDEBUG -g3
debug: compile_libs_debug $(EXEC)

compile_libs_debug:
	cd $(DIR_UTILS); make debug -s
	cd $(DIR_STRUCTURES); make debug -s
	cd $(DIR_PARSER); make debug -s
	cd $(DIR_SERIALIZER_HTML); make debug -s
	cd $(DIR_VARIOUS); make debug -s
