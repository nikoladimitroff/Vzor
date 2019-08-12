#include "vzor/vzor.h"
#include "doctest/doctest.h"
#include "TestHelpers.h"
#include "CustomContainers.h"
#include "GeometryPrimitives.h"

SCENARIO("Templated types are reflected accurately")
{
	GIVEN("a custom vector class")
	{
		THEN("it itself is reflected accurately")
		{
			const auto& vectorOfInts = Vzor::TypeOf<CustomVector<int>>();
			CHECK_EQ(vectorOfInts.Name, "CustomVector");

			TestHelpers::CheckType<CustomVector<int>>("CustomVector",
				{
					{"capacity_", Vzor::TypeIdOf<size_t>()},
					{"size_", Vzor::TypeIdOf<size_t>()},
				});
		}
		THEN("all of its instantiations have the same typeid")
		{
			const auto& vectorOfInts = Vzor::TypeOf<CustomVector<int>>();
			const auto& vectorOfChars = Vzor::TypeOf<CustomVector<char>>();
			const auto& vectorOfQuaternions = Vzor::TypeOf<CustomVector<Quaternion>>();
			CHECK_EQ(vectorOfInts.TypeId, vectorOfChars.TypeId);
			CHECK_EQ(vectorOfInts.TypeId, vectorOfQuaternions.TypeId);
		}
	}

}
