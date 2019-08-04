#pragma once

class [[reflect::type]] Animal
{
	[[reflect::data]]
	int Identifier;
};

class [[reflect::type]] Mammal : public Animal
{
	[[reflect::data]]
	int PregnancyLength;
};

class [[reflect::type]] Canine : public Mammal
{
	[[reflect::data]]
	int PreyIdentifier;
};

class [[reflect::type]] Dog : public Canine
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