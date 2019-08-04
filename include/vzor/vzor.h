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
		bool IsValid() const
		{
			return Name != nullptr;
		}
		operator bool() const
		{
			return IsValid();
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

	using TypeId = unsigned int;

	template<class T>
	constexpr TypeId TypeIdOf()
	{
		assert(false);
	}

	#define SPECIALIZE(Type, Value) \
	template<> constexpr TypeId TypeIdOf<Type>() { return Value; }

	namespace Detail
	{
		// TODO IMPORTANT: AUTO FILL PRIMITIVE TYPES
		extern const ReflectedType* AllReflectedTypes;

#ifdef VZOR_IMPLEMENT
		const ReflectedType* Vzor::Detail::AllReflectedTypes = nullptr;
#endif
	}

	template<typename T>
	inline const ReflectedType& TypeOf()
	{
		return Detail::AllReflectedTypes[TypeIdOf<T>()];
	}

	inline const ReflectedType& TypeOf(int typeId)
	{
		return Detail::AllReflectedTypes[typeId];
	}
};