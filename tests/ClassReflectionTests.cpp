#include "vzor/vzor.h"
#include "vzor/vzor_attributes.h"
#include "doctest/doctest.h"
#include "GeometryPrimitives.h"
#include "TransformData.h"
#include "AnimalHierarchy.h"

namespace VarFlags
{
	enum VarFlags
	{
		None = 0x0,
		IsRef = 0x1,
		IsConst = 0x2,
		IsPointer = 0x4
	};
}

struct ReflectedVarDescription
{
	const char* Name;
	const Vzor::TypeIdentifier Type;
	const VarFlags::VarFlags Flags;
};

void CheckVariable(const Vzor::ReflectedVariable& var, const ReflectedVarDescription& expected)
{
	REQUIRE(var.IsValid());
	CHECK_EQ(var.Name, expected.Name);
	CHECK_EQ(var.TypeId, expected.Type);
	CHECK_EQ(var.IsRef, bool(expected.Flags & VarFlags::IsRef));
	CHECK_EQ(var.IsConst, bool(expected.Flags & VarFlags::IsConst));
	CHECK_EQ(var.IsPointer(), bool(expected.Flags & VarFlags::IsPointer));
}

template<typename T>
void CheckType(const char* expectedName,
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

	//const auto baseCount = std::count_if(
	//	typeInfo.BaseTypes.begin(), typeInfo.BaseTypes.end(), [](auto& m) { return m != Vzor::InvalidTypeIdentifier; });
	//REQUIRE_EQ(baseCount, expectedBaseTypes.size());
	//for (int i = 0; i < baseCount; i++)
	//{
	//	CHECK_EQ(typeInfo.BaseTypes[i], *(expectedBaseTypes.begin() + i));
	//}
}


template<typename T>
void CheckType(const char* expectedName,
	std::initializer_list<ReflectedVarDescription> expectedMembers)
{
	CheckType<T>(expectedName, expectedMembers, {});
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

	GIVEN("A complex class hierarchy")
	{
		AND_THEN("A base class")
		{
			CheckType<Animal>("Animal",
			{
				{"AnimalId", Vzor::TypeIdOf<int>()},
			});
		}
		AND_THEN("A bunch of subclasses")
		{
			CheckType<Mammal>("Mammal",
			{
				{"MammalPregnancyLength", Vzor::TypeIdOf<int>()},
			},
				{Vzor::TypeIdOf<Animal>()}
			);
			CheckType<Canine>("Canine",
				{
					{"CaninePreyIdentifier", Vzor::TypeIdOf<int>()},
				},
				{ Vzor::TypeIdOf<Mammal>() }
			);
			CheckType<Dog>("Dog",
				{
					{"DogBreedIdentifier", Vzor::TypeIdOf<int>()},
				},
				{ Vzor::TypeIdOf<Canine>() }
			);
			CheckType<Wolf>("Wolf",
				{
					{"IsAlpha", Vzor::TypeIdOf<bool>()},
				},
				{ Vzor::TypeIdOf<Canine>() }
			);
		}
	}

	GIVEN("A user class with pointers and refs")
	{
		CheckType<TransformManager>("TransformManager",
			{
				{"GlobalTranslationOffset", Vzor::TypeIdOf<Vector3>(), static_cast<VarFlags::VarFlags>(VarFlags::IsConst | VarFlags::IsRef)},
				{"PlayerTransform", Vzor::TypeIdOf<TransformData>(), VarFlags::IsPointer},
			});
	}

	GIVEN("A user class with EnabledReflectionFromThis")
	{
		TransformData data;
		const auto& typeFromInstance = Vzor::TypeOf(data);
		const auto& typeFromVzor = Vzor::TypeOf<TransformData>();
		CHECK_EQ(typeFromInstance, typeFromVzor);
	}

	GIVEN("A user hierarchy with multiple parents deriving from EnabledReflectionFromThis")
	{
		Dog dog;
		const Canine& canine = dog;
		const Mammal* mammal = &dog;
		const auto& typeFromChild = Vzor::TypeOf(dog);
		const auto& typeFromParent = Vzor::TypeOf(canine);
		const auto& typeFromGrandparent = Vzor::TypeOf(*mammal);
		CHECK_EQ(typeFromChild, typeFromParent);
		CHECK_EQ(typeFromChild, typeFromGrandparent);
	}

	GIVEN("Reading the value from objects produces correct results")
	{
		GIVEN("a class without EnableReflectionFromThis")
		{
			Vector3 vec = { 2.f, 3.f, 5.f };
			const auto& typeOfVec = Vzor::TypeOf<Vector3>();
			CHECK_EQ(vec.X, *typeOfVec.DataMembers[0].ReadMemoryAs<float>(&vec));
			CHECK_EQ(vec.Y, *typeOfVec.DataMembers[1].ReadMemoryAs<float>(&vec));
			CHECK_EQ(vec.Z, *typeOfVec.DataMembers[2].ReadMemoryAs<float>(&vec));
		}
		GIVEN("a class with lots of bases")
		{
			Dog doggy;
			srand(42);
			doggy.AnimalId = rand();
			doggy.MammalPregnancyLength = rand();
			doggy.CaninePreyIdentifier = rand();
			doggy.DogBreedIdentifier = rand();
			const auto& typeOfDog = Vzor::TypeOf<Dog>();
			CHECK_EQ(doggy.DogBreedIdentifier, *typeOfDog.DataMembers[0].ReadMemoryAs<int>(&doggy));
			CHECK_EQ(doggy.CaninePreyIdentifier, *typeOfDog.GetBaseAtIndex(0).DataMembers[0].ReadMemoryAs<int>(&doggy));
			CHECK_EQ(doggy.MammalPregnancyLength, *typeOfDog.GetBaseAtIndex(0).GetBaseAtIndex(0).DataMembers[0].ReadMemoryAs<int>(&doggy));
			CHECK_EQ(doggy.AnimalId, *typeOfDog.GetBaseAtIndex(0).GetBaseAtIndex(0).GetBaseAtIndex(0).DataMembers[0].ReadMemoryAs<int>(&doggy));
		}
	}

	GIVEN("Property attributes are parsed correctly")
	{
		GIVEN("types are passed correctly")
		{
			const auto& typeOfVec = Vzor::TypeOf<Vector3>();
			CHECK_EQ(typeOfVec.DataMembers[0].Attributes[0].Type, Vzor::PropertyAttribute::Foo);
			CHECK_EQ(typeOfVec.DataMembers[0].Attributes[1].Type, Vzor::PropertyAttribute::Bar);
			CHECK_EQ(typeOfVec.DataMembers[0].Attributes[2].Type, Vzor::PropertyAttribute::Baz);
			CHECK_EQ(typeOfVec.DataMembers[0].Attributes[3].Type, Vzor::PropertyAttribute::Min);
		}
		GIVEN("args are passed correctly")
		{
			const auto& typeOfVec = Vzor::TypeOf<Vector3>();
			CHECK_EQ(typeOfVec.DataMembers[0].Attributes[0].Type, Vzor::PropertyAttribute::Foo);
			CHECK_EQ(typeOfVec.DataMembers[0].Attributes[0].Args[0].AsString(), "jj");
			CHECK_EQ(typeOfVec.DataMembers[0].Attributes[0].Args[1].AsInteger(), 7);
			CHECK_EQ(typeOfVec.DataMembers[0].Attributes[0].Args[2].AsInteger(), 8);
			CHECK_EQ(typeOfVec.DataMembers[0].Attributes[3].Type, Vzor::PropertyAttribute::Min);
			CHECK_EQ(typeOfVec.DataMembers[0].Attributes[3].Args[0].AsDouble(), 5.55);
		}
	}
}

