#pragma once

// These classes store their names in their members to make it more obvious in tests
// which member comes from which base

class [[reflect::type]] Animal
{
public:
	[[reflect::data]]
	int AnimalId;
};

class [[reflect::type]] Mammal : public Animal, public Vzor::EnableReflectionFromThis<Mammal>
{
public:
	[[reflect::data]]
	int MammalPregnancyLength;
};

class [[reflect::type]] Canine : public Mammal, public Vzor::EnableReflectionFromThis<Canine, Vzor::InheritedReflection::BaseClassAlreadyReflected>
{
public:
	[[reflect::data]]
	int CaninePreyIdentifier;
};

class [[reflect::type]] Dog : public Canine, public Vzor::EnableReflectionFromThis<Dog, Vzor::InheritedReflection::BaseClassAlreadyReflected>
{
public:
	[[reflect::data]]
	int DogBreedIdentifier;
};

class [[reflect::type]] Wolf : public Canine
{
public:
	[[reflect::data]]
	bool IsAlpha;
};


#include "vzorgenerated/AnimalHierarchy.h"
