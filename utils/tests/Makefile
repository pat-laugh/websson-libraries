# MIT License
# Copyright 2017 Patrick Laughrea

DIR_PROJECT = ../..
DIR_UTILS = $(DIR_PROJECT)/utils
DIR_VARIOUS = $(DIR_PROJECT)/various

OPTIONS = -std=c++11 -Wall -Wextra -Wno-missing-field-initializers -I $(DIR_PROJECT)
CPP_FILES = $(wildcard *.cpp)
OBJ_FILES = $(notdir $(CPP_FILES:.cpp=.o))
EXEC = tests.out

all: compile_libs $(EXEC)

compile_libs:
	cd $(DIR_UTILS); make -s
	cd $(DIR_VARIOUS); make -s

$(EXEC): $(OBJ_FILES)
	$(CXX) -o $@ $^ -L$(DIR_UTILS) -lutils -L$(DIR_VARIOUS) -lvarious

%.o: %.cpp
	$(CXX) $(OPTIONS) -o $@ -c $<

clean:
	cd $(DIR_UTILS); make clean -s
	cd $(DIR_VARIOUS); make clean -s
	rm -rf $(EXEC) *.o

test:
	make all -s && ./$(EXEC)