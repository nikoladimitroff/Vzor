#pragma once
#include <vector>

struct [[reflect::type]] Vector3
{
	[[reflect::data]]
	float X;
	[[reflect::data]]
	float Y;
	[[reflect::data]]
	float Z;
};

//[[reflect::type]]
//class TransformManager : public ComponentManager
//{
// TODO: Store all 3 vectors in sequential memory
//[[reflect::data(SoA)]]
//stl::vector<Vector3> m_Positions;
//[[reflect::data]]
//stl::vector<Quaternion> m_Rotations;
//[[reflect::data]]
//stl::vector<Vector3> m_Scales;
//
//[[reflect::data]]
//stl::unordered_map<EntityId, EntityId::IndexType> m_EntityToIndex;
//friend struct TransformInstance;
//};

#include "vzorgenerated/TestTypes.h"