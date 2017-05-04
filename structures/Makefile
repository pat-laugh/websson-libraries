# MIT License
# Copyright(c) 2017 Patrick Laughrea

CC = g++
OPTIONS = -std=c++11 -Wall -Wextra -Wno-missing-field-initializers
LIBRARY = libstructures.a

all: $(LIBRARY)

$(LIBRARY): keywords.o webss.o
	ar -rc $(LIBRARY) $^

keywords.o: keywords.cpp
	$(CC) $(OPTIONS) -o $@ -c $<

webss.o: webss.cpp
	$(CC) $(OPTIONS) -o $@ -c $<

clean:
	rm -rf $(LIBRARY) *.o