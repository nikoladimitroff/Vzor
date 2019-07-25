#pragma once
// TODO: Remove dependency
#include <array>
#include <cassert>

namespace Vzor
{
	class ReflectedVariable
	{
	public:
		ReflectedVariable()
		{}
		ReflectedVariable(const int id, const char* name, const void* offset)
			: TypeId(id), Name(name), OffsetToBase(offset)
		{}
		operator bool() const
		{
			return Name != nullptr;
		}
		const int TypeId = -1;
		const char* Name = nullptr;
		const void* OffsetToBase = 0x0;
	};

	class ReflectedType
	{
	public:
		int TypeId;
		const char* Name = "";
		std::array<ReflectedVariable, 32> DataMembers;
	};


	template<class T>
	constexpr int TypeIdOf()
	{
		assert(false);
	}

	#define SPECIALIZE(Type, Value) \
	template<> constexpr int TypeIdOf<Type>() { return Value; }

	namespace Detail
	{
		// TODO IMPORTANT: AUTO FILL PRIMITIVE TYPES
		extern ReflectedType* AllReflectedTypes;

#ifdef VZOR_IMPLEMENT
		ReflectedType* Vzor::Detail::AllReflectedTypes = nullptr;
#endif
	}

	template<typename T>
	constexpr ReflectedType& GetTypeInfo()
	{
		return Detail::AllReflectedTypes[TypeIdOf<T>()];
	}

	constexpr ReflectedType& GetTypeInfo(int typeId)
	{
		return Detail::AllReflectedTypes[typeId];
	}
};