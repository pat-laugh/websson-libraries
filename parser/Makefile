# MIT License
# Copyright(c) 2017 Patrick Laughrea

CC = g++
OPTIONS = -std=c++11 -Wall -Wextra -Wno-missing-field-initializers -iquote ..
LIBRARY = libparser.a

all: $(LIBRARY)

$(LIBRARY): binaryIterator.o curl.o entityManager.o importManager.o paramDocumentIncluder.o parser.o parserBinary.o parserContainers.o parserEntities.o parserKeyValues.o parserNumbers.o parserStrings.o parserTemplateHead.o parserTemplates.o unicode.o utilsNumbers.o utilsSweepers.o
	ar -rc $(LIBRARY) $^

binaryIterator.o: binaryIterator.cpp
	$(CC) $(OPTIONS) -o $@ -c $<

curl.o: curl.cpp
	$(CC) $(OPTIONS) -o $@ -c $<

entityManager.o: entityManager.cpp
	$(CC) $(OPTIONS) -o $@ -c $<

importManager.o: importManager.cpp
	$(CC) $(OPTIONS) -o $@ -c $<

paramDocumentIncluder.o: paramDocumentIncluder.cpp
	$(CC) $(OPTIONS) -o $@ -c $<

parser.o: parser.cpp
	$(CC) $(OPTIONS) -o $@ -c $<

parserBinary.o: parserBinary.cpp
	$(CC) $(OPTIONS) -o $@ -c $<

parserContainers.o: parserContainers.cpp
	$(CC) $(OPTIONS) -o $@ -c $<

parserEntities.o: parserEntities.cpp
	$(CC) $(OPTIONS) -o $@ -c $<

parserKeyValues.o: parserKeyValues.cpp
	$(CC) $(OPTIONS) -o $@ -c $<

parserNumbers.o: parserNumbers.cpp
	$(CC) $(OPTIONS) -o $@ -c $<

parserStrings.o: parserStrings.cpp
	$(CC) $(OPTIONS) -o $@ -c $<

parserTemplateHead.o: parserTemplateHead.cpp
	$(CC) $(OPTIONS) -o $@ -c $<

parserTemplates.o: parserTemplates.cpp
	$(CC) $(OPTIONS) -o $@ -c $<

unicode.o: unicode.cpp
	$(CC) $(OPTIONS) -o $@ -c $<

utilsNumbers.o: utilsNumbers.cpp
	$(CC) $(OPTIONS) -o $@ -c $<

utilsSweepers.o: utilsSweepers.cpp
	$(CC) $(OPTIONS) -o $@ -c $<

clean:
	rm -rf $(LIBRARY) *.o