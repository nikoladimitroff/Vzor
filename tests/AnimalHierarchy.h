#pragma once

class [[reflect::type]] Animal
{
	[[reflect::data]]
	int Identifier;
};

class [[reflect::type]] Mammal : public Animal, public virtual Vzor::EnableReflectionFromThis<Mammal>
{
	[[reflect::data]]
	int PregnancyLength;
};

class [[reflect::type]] Canine : public Mammal, public virtual Vzor::EnableReflectionFromThis<Canine>
{
	[[reflect::data]]
	int PreyIdentifier;
};

class [[reflect::type]] Dog : public Canine, public virtual Vzor::EnableReflectionFromThis<Dog>
{
	[[reflect::data]]
	int BreedIdentifier;
};

class [[reflect::type]] Wolf : public Canine
{
	[[reflect::data]]
	bool IsAlpha;
};


#include "vzorgenerated/AnimalHierarchy.h"