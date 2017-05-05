# MIT License
# Copyright(c) 2017 Patrick Laughrea

OPTIONS = -std=c++11 -Wall -Wextra -Wno-missing-field-initializers -iquote ..
LIBRARY = libparser.a

all: $(LIBRARY)

$(LIBRARY): binaryIterator.o curl.o entityManager.o importManager.o paramDocumentIncluder.o parser.o parserBinary.o parserContainers.o parserEntities.o parserKeyValues.o parserNumbers.o parserStrings.o parserTemplateHead.o parserTemplates.o unicode.o utilsNumbers.o utilsSweepers.o
	ar -rc $(LIBRARY) $^

binaryIterator.o: binaryIterator.cpp
	$(CXX) $(OPTIONS) -o $@ -c $<

curl.o: curl.cpp
	$(CXX) $(OPTIONS) -o $@ -c $<

entityManager.o: entityManager.cpp
	$(CXX) $(OPTIONS) -o $@ -c $<

importManager.o: importManager.cpp
	$(CXX) $(OPTIONS) -o $@ -c $<

paramDocumentIncluder.o: paramDocumentIncluder.cpp
	$(CXX) $(OPTIONS) -o $@ -c $<

parser.o: parser.cpp
	$(CXX) $(OPTIONS) -o $@ -c $<

parserBinary.o: parserBinary.cpp
	$(CXX) $(OPTIONS) -o $@ -c $<

parserContainers.o: parserContainers.cpp
	$(CXX) $(OPTIONS) -o $@ -c $<

parserEntities.o: parserEntities.cpp
	$(CXX) $(OPTIONS) -o $@ -c $<

parserKeyValues.o: parserKeyValues.cpp
	$(CXX) $(OPTIONS) -o $@ -c $<

parserNumbers.o: parserNumbers.cpp
	$(CXX) $(OPTIONS) -o $@ -c $<

parserStrings.o: parserStrings.cpp
	$(CXX) $(OPTIONS) -o $@ -c $<

parserTemplateHead.o: parserTemplateHead.cpp
	$(CXX) $(OPTIONS) -o $@ -c $<

parserTemplates.o: parserTemplates.cpp
	$(CXX) $(OPTIONS) -o $@ -c $<

unicode.o: unicode.cpp
	$(CXX) $(OPTIONS) -o $@ -c $<

utilsNumbers.o: utilsNumbers.cpp
	$(CXX) $(OPTIONS) -o $@ -c $<

utilsSweepers.o: utilsSweepers.cpp
	$(CXX) $(OPTIONS) -o $@ -c $<

clean:
	rm -rf $(LIBRARY) *.o

debug: OPTIONS += -DNDEBUG -g3
debug: all