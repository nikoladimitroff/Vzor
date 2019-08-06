# Vzor

Vzor is a reflection library for C++, with interface in Python and C++, macroless, intrusiveless, working on top of the modern C++ attributes. ATM the library is close to unusable but come back in some time to check.

## Requirements

* C++11 compatible compiler
* Python 3.4

## What it can do currently

* Reflect a folder of C++ header files passed to the generator python script
* Generate descriptions of the any parsed file in the headers as C++ objects
* Provides a simple API to query properties of any reflected types.

See the tests project for examples.

## Missing features

1. Templates
2. Reflecting nested classes
3. No support for types in different namespaces with the same names.
4. No docs
5. Pointer and reference support
6. Performance / stress / sanity tests
7. Support for accessing actual property / function values (atm you can see a description of the properties, but can't access their value)
8. Support for functions
9. Support for enums
10. Better error reporting
