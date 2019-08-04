#pragma once
#include "GeometryPrimitives.h"

class [[reflect::type]] TransformData
{
	[[reflect::data]]
	Quaternion Rotation;
	[[reflect::data]]
	Vector3 Translation;
	[[reflect::data]]
	float Scale;
};

#include "vzorgenerated/TransformData.h"