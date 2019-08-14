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

class [[reflect::type]] TransformManager
{
	[[reflect::data]]
	const Vector3& GlobalTranslationOffset;
	[[reflect::data]]
	TransformData* PlayerTransform;
};

#include "vzorgenerated/TransformData.h" 