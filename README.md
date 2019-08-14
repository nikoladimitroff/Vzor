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

1. Reflecting nested classes
1. No support for types in different namespaces with the same names.
1. No docs
1. No support for const in pointers (e.g. `Foo const *`)
1. Performance / stress / sanity tests
1. Support for accessing actual property / function values (atm you can see a description of the properties, but can't access their value)
1. Support for functions
1. Support for enums
1. Better error reporting
