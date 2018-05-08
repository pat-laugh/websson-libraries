# All rights reserved
# Copyright 2018 Patrick Laughrea

DIR_PROJECT = ..
DIR_UTILS = $(DIR_PROJECT)/utils
DIR_STRUCTURES = $(DIR_PROJECT)/structures
DIR_PARSER = $(DIR_PROJECT)/parser
DIR_VARIOUS = $(DIR_PROJECT)/various

OPTIONS = -std=c++11 -Wall -Wextra -Wno-missing-field-initializers -I $(DIR_PROJECT) -DCOMPILE_WEBSS
CPP_FILES = $(wildcard *.cpp)
OBJ_FILES = $(notdir $(CPP_FILES:.cpp=.o))
EXEC = webss.out

all: compile_libs $(EXEC)

disable_import: DISABLE_IMPORT = 1
disable_import: compile_libs $(EXEC)

compile_libs:
	cd $(DIR_UTILS); make -s
	cd $(DIR_STRUCTURES); make webssc -s
	cd $(DIR_PARSER); \
	if [ $(DISABLE_IMPORT) ]; then \
		make webssc_disable_import -s; \
	else \
		make webssc -s; \
	fi
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
	-L$(DIR_STRUCTURES) -lstructures -L$(DIR_UTILS) -lutils -L$(DIR_VARIOUS) -lvarious

%.o: %.cpp
	$(CXX) $(OPTIONS) -o $@ -c $<

clean:
	cd $(DIR_UTILS); make clean -s
	cd $(DIR_STRUCTURES); make clean -s
	cd $(DIR_PARSER); make clean -s
	cd $(DIR_VARIOUS); make clean -s
	rm -rf $(EXEC) *.o
