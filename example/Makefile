# This file is in the public domain.
all:
	mkdir -p generated
	schematicpp generated example.xsd
	g++ example.cpp generated/*.cpp -lschematicpp -lxerces-c -o example
