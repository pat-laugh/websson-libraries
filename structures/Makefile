# MIT License
# Copyright 2017-2018 Patrick Laughrea

DIR_PROJECT = ..
DIR_VARIOUS = $(DIR_PROJECT)/various

IS_MAC = $$(test "$$(uname -s)" = 'Darwin' && echo 1 || echo 0)
DYNAMIC_OPT = $$(test $(IS_MAC) -eq 1 && echo '-dynamiclib' || echo '-shared')

OPTIONS = -std=c++11 -Wall -Wextra -Wno-missing-field-initializers -I $(DIR_PROJECT)
STATIC_LIBRARY = libstructures.a
SHARED_LIBRARY = libstructures.so
CPP_FILES = $(wildcard *.cpp)
OBJ_FILES = $(notdir $(CPP_FILES:.cpp=.o))

all: $(STATIC_LIBRARY) $(SHARED_LIBRARY)

$(STATIC_LIBRARY): $(OBJ_FILES)
	ar -rcs $@ $^

$(SHARED_LIBRARY): $(STATIC_LIBRARY)
	cd $(DIR_VARIOUS); make -s
	$(CXX) -o $@ $(DYNAMIC_OPT) -Wl,-all_load $(STATIC_LIBRARY) -L$(DIR_VARIOUS) -lvarious
	test $(IS_MAC) -eq 1 && \
			for s in various; do \
				install_name_tool -change lib$$s.so $$(pwd)/../$$s/lib$$s.so $(SHARED_LIBRARY); \
			done

%.o: %.cpp
	$(CXX) $(OPTIONS) -o $@ -c $<

clean:
	cd $(DIR_VARIOUS); make clean -s
	rm -rf $(STATIC_LIBRARY) $(SHARED_LIBRARY) *.o

debug: OPTIONS += -DNDEBUG -g3
debug: all