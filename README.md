# Vzor
Vzor is a reflection library for C++, with interface in Python and C++, macroless, intrusiveless, working on top modern C++ attributes. Unusable at the moment, do not even try to use it.

# Unsupported C++

1. Templates
2. Reflecting nested classes
3. No support for types in different namespaces with the same names.
4. All types must be included in the generated dat file? Do they?

# Big upcoming TODOS

1. Fix generated headers.
2. Add prebuild steps to run generation script.
3. Include generated headers at end or in pch.
4. Add some tests before moving on to newer features.
5. Fix offset to members support.
5. Fix template, container and pointer support.
6. Fix namespace support.
