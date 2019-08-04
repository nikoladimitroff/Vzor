#include "vzor/vzor.h"
#include "doctest/doctest.h"
#include "GeometryPrimitives.h"
#include "TransformData.h"

struct ReflectedVarDescription
{
	const char* Name;
	Vzor::TypeId Type;
};

void CheckVariable(const Vzor::ReflectedVariable& var, const ReflectedVarDescription& expected)
{
	REQUIRE(var.IsValid());
	CHECK_EQ(var.Name, expected.Name);
	CHECK_EQ(var.TypeId, expected.Type);
}

template<typename T>
void CheckType(const char* expectedName, std::initializer_list<ReflectedVarDescription> expectedMembers)
{
	const Vzor::ReflectedType& typeInfo = Vzor::TypeOf<T>();

	REQUIRE_EQ(typeInfo.TypeId, Vzor::TypeIdOf<T>());
	REQUIRE_EQ(typeInfo.Name, expectedName);

	const auto memberCount = std::count_if(
		typeInfo.DataMembers.begin(), typeInfo.DataMembers.end(), [](auto& m) { return m; });
	REQUIRE_EQ(memberCount, expectedMembers.size());
	for (int i = 0; i < memberCount; i++)
	{
		CheckVariable(typeInfo.DataMembers[i], *(expectedMembers.begin() + i));
	}
}

SCENARIO("Types are reflected accurately")
{
	GIVEN("A trivial user class")
	{
		CheckType<Vector3>("Vector3",
		{
			{"X", Vzor::TypeIdOf<float>()},
			{"Y", Vzor::TypeIdOf<float>()},
			{"Z", Vzor::TypeIdOf<float>()},
		});
	}
	GIVEN("A 2nd trivial user class in the same file")
	{
		CheckType<Quaternion>("Quaternion",
			{
				{"X", Vzor::TypeIdOf<float>()},
				{"Y", Vzor::TypeIdOf<float>()},
				{"Z", Vzor::TypeIdOf<float>()},
				{"W", Vzor::TypeIdOf<float>()},
			});
	}

	GIVEN("A user class using another user class in a separate file")
	{
		CheckType<TransformData>("TransformData",
			{
				{"Rotation", Vzor::TypeIdOf<Quaternion>()},
				{"Translation", Vzor::TypeIdOf<Vector3>()},
				{"Scale", Vzor::TypeIdOf<float>()},
			});
	}
}

