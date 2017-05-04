# MIT License
# Copyright(c) 2017 Patrick Laughrea

CC = g++
OPTIONS = -std=c++11 -Wall -Wextra -Wno-missing-field-initializers -iquote ..
LIBRARY = libserializer.a

all: $(LIBRARY)

$(LIBRARY): serializer.o serializerBinary.o
	ar -rc $(LIBRARY) $^

serializer.o: serializer.cpp
	$(CC) $(OPTIONS) -o $@ -c $<

serializerBinary.o: serializerBinary.cpp
	$(CC) $(OPTIONS) -o $@ -c $<

clean:
	rm -rf $(LIBRARY) *.o