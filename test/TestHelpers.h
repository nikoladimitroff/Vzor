#pragma once
#include "vzor/vzor.h"
#include "doctest/doctest.h"

namespace TestHelpers
{
	struct ReflectedVarDescription
	{
		const char* Name;
		Vzor::TypeIdentifier Type;
	};

	inline void CheckVariable(const Vzor::ReflectedVariable& var, const ReflectedVarDescription& expected)
	{
		REQUIRE(var.IsValid());
		CHECK_EQ(var.Name, expected.Name);
		CHECK_EQ(var.TypeId, expected.Type);
	}

	template<typename T>
	inline void CheckType(const char* expectedName,
		std::initializer_list<ReflectedVarDescription> expectedMembers,
		std::initializer_list<Vzor::TypeIdentifier> expectedBaseTypes)
	{
		const Vzor::ReflectedType& typeInfo = Vzor::TypeOf<T>();

		REQUIRE(typeInfo.IsValid());
		REQUIRE_EQ(typeInfo.TypeId, Vzor::TypeIdOf<T>());
		REQUIRE_EQ(typeInfo.Name, expectedName);

		const auto memberCount = std::count_if(
			typeInfo.DataMembers.begin(), typeInfo.DataMembers.end(), [](auto& m) { return m; });
		REQUIRE_EQ(memberCount, expectedMembers.size());
		for (int i = 0; i < memberCount; i++)
		{
			CheckVariable(typeInfo.DataMembers[i], *(expectedMembers.begin() + i));
		}

		const auto baseCount = std::count_if(
			typeInfo.BaseTypes.begin(), typeInfo.BaseTypes.end(), [](auto& m) { return m != Vzor::InvalidTypeIdentifier; });
		REQUIRE_EQ(baseCount, expectedBaseTypes.size());
		for (int i = 0; i < baseCount; i++)
		{
			CHECK_EQ(typeInfo.BaseTypes[i], *(expectedBaseTypes.begin() + i));
		}
	}


	template<typename T>
	inline void CheckType(const char* expectedName,
		std::initializer_list<ReflectedVarDescription> expectedMembers)
	{
		CheckType<T>(expectedName, expectedMembers, {});
	}
}
