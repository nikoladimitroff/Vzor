#pragma once
// TODO: Remove dependency
#include <array>
#include <cassert>
#include <type_traits>

namespace Vzor
{
	struct TypeIdentifier
	{
		constexpr TypeIdentifier()
			: Id(static_cast<uint32_t>(-1))
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
		constexpr uint32_t GetValue() const
		{
			return Id;
		}
	private:
		uint32_t Id;
	};
	constexpr TypeIdentifier InvalidTypeIdentifier = TypeIdentifier();

	class ReflectedVariable
	{
	public:
		ReflectedVariable()
			: IsConst(false)
			, IsRef(false)
		{}
		ReflectedVariable(const uint32_t id, const char* name, uint8_t pointerLevels, bool isConst, bool isRef, const void* offset)
			: TypeId(id), Name(name)
			, PointerLevels(pointerLevels), IsConst(isConst), IsRef(isRef)
			, OffsetToBase(offset)
		{}
		bool IsPointer() const
		{
			return PointerLevels > 0;
		}
		bool IsValid() const
		{
			return TypeId != InvalidTypeIdentifier;
		}
		operator bool() const
		{
			return IsValid();
		}
		const char* Name = nullptr;
		const void* OffsetToBase = 0x0;
		const TypeIdentifier TypeId = InvalidTypeIdentifier;
		const unsigned char PointerLevels = 0u;
		// Can't inline initialize bit fields until C++20
		const bool IsConst : 1;
		const bool IsRef : 1;
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

	namespace Detail
	{
		// TODO IMPORTANT: AUTO FILL PRIMITIVE TYPES
		extern const ReflectedType* AllReflectedTypes;

#ifdef VZOR_IMPLEMENT
		const ReflectedType* Vzor::Detail::AllReflectedTypes = nullptr;
#endif
	}

	template<typename T>
	// TODO: RENAME
	struct TypeIdS : std::integral_constant<unsigned int, (unsigned int)-1>
	{
	};

	template<typename T>
		constexpr
		TypeIdentifier
		TypeIdOf()
	{
		//assert(false);
		return TypeIdentifier(TypeIdS<T>::value);
	}

#define SPECIALIZE(Type, Value) \
	template<> \
	struct TypeIdS<Type> : public std::integral_constant<unsigned int, Value> \
	{};

#define SPECIALIZE_TEMPLATED(Type, Value) \
	template<typename InnerClass> \
	struct TypeIdS<Type<InnerClass>> : public std::integral_constant<unsigned int, Value> \
	{};

	template<typename T>
	inline const ReflectedType& TypeOf()
	{
		return Detail::AllReflectedTypes[TypeIdOf<T>().GetValue()];
	}

	inline const ReflectedType& TypeOf(TypeIdentifier typeId)
	{
		return Detail::AllReflectedTypes[typeId.GetValue()];
	}

	struct RuntimeReflectionInfo
	{
		TypeIdentifier ReflectedTypeId;
	};

	template<typename T>
	struct EnableReflectionFromThis : public virtual RuntimeReflectionInfo
	{
		EnableReflectionFromThis()
		{
			ReflectedTypeId = TypeIdOf<T>();
		}

		inline static const ReflectedType& StaticTypeOf()
		{
			return Vzor::TypeOf<T>();
		}
	};

	inline const ReflectedType& TypeOf(const RuntimeReflectionInfo& obj)
	{
		return Detail::AllReflectedTypes[obj.ReflectedTypeId.GetValue()];
	}
};
