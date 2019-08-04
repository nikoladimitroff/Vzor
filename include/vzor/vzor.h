#pragma once
// TODO: Remove dependency
#include <array>
#include <cassert>

namespace Vzor
{
	struct TypeIdentifier
	{
		constexpr TypeIdentifier()
			: Id(static_cast<unsigned int>(-1))
		{}
		explicit constexpr TypeIdentifier(unsigned int value)
			: Id(value)
		{}
		TypeIdentifier(const TypeIdentifier&) = default;
		TypeIdentifier& operator=(const TypeIdentifier&) = default;
		inline constexpr bool operator==(const TypeIdentifier other) const
		{
			return Id == other.Id;
		}
		inline constexpr bool operator!=(const TypeIdentifier other) const
		{
			return !(Id == other.Id);
		}
		constexpr unsigned int GetValue() const
		{
			return Id;
		}
	private:
		unsigned int Id;
	};
	constexpr TypeIdentifier InvalidTypeIdentifier = TypeIdentifier();

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
		const std::array<TypeIdentifier, 4> BaseTypes;
	};

	template<class T>
	constexpr TypeIdentifier TypeIdOf()
	{
		assert(false);
	}

	#define SPECIALIZE(Type, Value) \
	template<> constexpr TypeIdentifier TypeIdOf<Type>() { return TypeIdentifier(Value); }

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
		return Detail::AllReflectedTypes[TypeIdOf<T>().GetValue()];
	}

	inline const ReflectedType& TypeOf(TypeIdentifier typeId)
	{
		return Detail::AllReflectedTypes[typeId.GetValue()];
	}
};