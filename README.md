# Vzor

Vzor is a reflection library for C++. Unlike other existing reflection systems, this one focuses on preserving your code's readability, without adding macros and other intermediate languages. You use standard C++ attributes to mark types from reflection and you use standard C++ to query type info at runtime.

Vzor firstly runs a python program which extracts valuable information from your code and then generates a database-like file containing all of the information necessary, accessible from your C++.

The library is a work in progress - unstable and unusable. I am happy to hear your feedback, so do scroll below for an overview and let me know what you think (open an issue or contact me directly over email).

## Requirements

* C++11 compatible compiler
* Python 3.4

## Usage

Put `[[reflect::type]]` on types you want to reflect and `[[reflect::data]]` on data members:

```cpp
struct [[reflect::type]] Vector3
{
	[[reflect::data]]
	float X;
	[[reflect::data]]
	float Y;
	[[reflect::data]]
	float Z;
};
```

Then query information about your type:

```cpp
const Vzor::ReflectedType& vectorTypeInfo = Vzor::TypeOf<Vector3>();
printf("The name of type is %s.\n", vectorTypeInfo.Name);
for (int i = 0; i < 3; i++)
{
    printf("Its next member is named %s and of type %s",
        vectorTypeInfo.DataMembers[i].Name,
        Vzor::TypeOf(vectorTypeInfo.DataMembers[i].TypeId).Name);
}
```

You can also query query information about your type (as long as your type derives from `Vzor::EnableReflectionFromThis`):

```cpp
class [[reflect::type]] TransformData : public Vzor::EnableReflectionFromThis<TransformData>
{
	[[reflect::data]]
	Quaternion Rotation;
	[[reflect::data]]
	Vector3 Translation;
	[[reflect::data]]
	float Scale;
};
...

TransformData data;
const ReflectedType& typeFromInstance = Vzor::TypeOf(data);
printf("My obj is of type: %s", typeFromInstance.Name);
```

## Integration

* Download this repo. You'd need the *generator* and the *include* dirs.
* Include the *include/vzor/vzor.h* header in your files as you see fit.
* Run `python generator/vzor.py` passing the folders you'd like reflected. Any *\*.h* / *\*.hpp* files
will be reflected.
* This will create one extra header file for each reflected header (*vzorgenerated/X.h* for every X.h in the original
dir. It will also generated a single *VzorDatabase.cpp* file.
* Add `#include "vzorgenerated/Foo.h"` as the last line of *Foo.h* in every reflected header, e.g.
```cpp
// If this is vector3.h, define your class
struct [[reflect::type]] Vector3
{
	[[reflect::data]]
	float X;
	[[reflect::data]]
	float Y;
	[[reflect::data]]
	float Z;
};
// And then include the generated file
#include "vzorgenerated/Vector3.h"
```
* Add *vzorgenerated/VzorDatabase.cpp* to your own project. This file contains the bulk of the information
and requires you to compile it alongside your assembly.

## What it can do currently

* Reflect a folder of C++ header files passed to the generator python script
* Generate descriptions of your types from headers
* Provides a simple API to query properties of any reflected types.

Explore the public API to learn more, which right now consits of these:
- `Vzor::ReflectedType` - stores data for some type
- `Vzor::ReflectedVariable` - stores data for some variable (usually a data member of a type)
- `Vzor::TypeOf` - an overloaded freestanding function which let's you statically query the type of a class
- `Vzor::EnableReflectionFromThis` - the class you should be inheriting from if you want your objects to know their own types

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
