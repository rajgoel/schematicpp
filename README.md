# schematicpp
## A simple XML schema compiler for C++, written in C++.

Forked from https://github.com/Tjoppen/james on June 6, 2023

## Purpose/goals
The purpose of this program is to transform a subset of XML schema defintions into C++ code for marshalling and unmarshalling documents conforming to one or more schemas. The generated code should not be needlessly complicated (no getters or setters). The number of dependencies should be kept to a minimum.

## Dependencies

schematicpp requires Xerces-C++ 3.2.x. On Ubuntu Linux Xerces can be installed by
```sh
sudo apt install libxerces-c-dev
```

## Build the program

The program is built like a typical CMake project. A normal build will look something like this (output omitted):

```sh
 ~/schematicpp$ mkdir build
 ~/schematicpp$ cd build
 ~/schematicpp/build$ cmake ..
 ~/schematicpp/build$ make
```

## A short guide to usage

Running the program without arguments produces the following usage information:

```
USAGE: schematicpp [-v] [-s] -p <projectname> -o <output-dir> -i <schema_1> ... <schema_n>
 -v	Verbose mode
 -s	Simulate generation but don't write anything to disk
 -p	Provide project name
 -o	Provide output directory
 -i	Provide list of XML schema definition files

 Generates C++ classes for marshalling and unmarshalling XML to C++ objects according to the given schemas.
```

The program parses the XML schema definition files in the given order and creates the files `<type>.cpp` and `<type>.h` for each type defined. These files are stored in the `classes/` subdirectory of the specified output directory. All of these classes are derived from a base class `XMLObject`. The program attempts to copy the files `XMLObject.cpp` and `XMLObject.h` from the `lib` directory of the program to the the `classes/` subdirectory of the specified output directory. If the program cannot find these files, a warning is displayed and the files must be copied manually.

A CMake configuration file is generated and placed in the specified output directory allowing to build a library from the generated classes.

## Build the library

The library can be built like a typical CMake project. A normal build will look something like this (output omitted):

```sh
 ~/schematicpp/outputdir$ mkdir build
 ~/schematicpp/outputdir$ cd build
 ~/schematicpp/outputdir/build$ cmake ..
 ~/schematicpp/outputdir/build$ make
```

When building a library, the given project name is used as the name of the library. A subdirectory `lib` is generated in which a single header file `<projectname>.h` and the library `lib<projectname>.a` are placed.

It is possible to specify a source file and the name of the executable to using the flags `-DSRC=<pathtosource>` and `-DEXE=<nameofexecutable>`. The following built
```sh
~/schematicpp/outputdir/build$ cmake -DSRC=../main.cpp -DEXE=xmlParser ..
~/schematicpp/outputdir/build$ make
```
creates an executable `xmlParser` using the source file `main.cpp` in the output directory. 
