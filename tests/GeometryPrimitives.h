#pragma once

struct [[reflect::type]] Vector3
{
	[[reflect::data]]
	float X;
	[[reflect::data]]
	float Y;
	[[reflect::data]]
	float Z;
};

struct [[reflect::type]] Quaternion
{
	[[reflect::data]]
	float X;
	[[reflect::data]]
	float Y;
	[[reflect::data]]
	float Z;
	[[reflect::data]]
	float W;
};

#include "vzorgenerated/GeometryPrimitives.h"