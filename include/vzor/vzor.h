#pragma once
// TODO: Remove dependency
#include <array>
#include <cassert>

namespace Vzor
{
	using TypeIdentifier = unsigned int;
	constexpr TypeIdentifier InvalidTypeIdentifier = static_cast<TypeIdentifier>(-1);

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
			return TypeId != InvalidTypeIdentifier;
		}
		operator bool() const
		{
			return IsValid();
		}
		const TypeIdentifier TypeId = InvalidTypeIdentifier;
		const char* Name = nullptr;
		const void* OffsetToBase = 0x0;
	};

	class ReflectedType
	{
	public:
		bool IsValid() const
		{
			return TypeId != InvalidTypeIdentifier;
		}
		operator bool() const
		{
			return IsValid();
		}

		const TypeIdentifier TypeId = InvalidTypeIdentifier;
		const char* Name = nullptr;
		const std::array<ReflectedVariable, 32> DataMembers;
	};

	template<class T>
	constexpr TypeIdentifier TypeIdOf()
	{
		assert(false);
	}

	#define SPECIALIZE(Type, Value) \
	template<> constexpr TypeIdentifier TypeIdOf<Type>() { return Value; }

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