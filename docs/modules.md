# Modules

To improve compile times cbuild implements a system to automatically
modularize the project.

## Compilation overview

Compiling cbuild projects begins with the `build.cpp`. The build file
contains the instructions to compile the project. The instructions are
derived from the `CBuild::Binary`. Due to the case of the project 
using mulitple libraries the compilation between each binary must 
remain seperate.

## Modularization

Modularization begins at the entry point of the binary. Modules are 
constructed by tokenizing and parsing c++ files to get all definitions,
functions, structures, and type definitions. When a module contains an
`#include` the included function is modularized first then the rest
of the file is parsed waiting for included functions to be used. This
process could generate warnings for unused functions
