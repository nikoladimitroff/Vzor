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

	struct AttributeArgument
	{
		enum class Type
		{
			Invalid,
			String,
			Double,
			Int,
			Bool,
			Compound,
			Token
		};
		AttributeArgument()
			: ValueType(Type::Invalid)
		{}
		AttributeArgument(int64_t value)
			: ValueType(Type::Int)
			, IntegerValue(value)
		{}
		AttributeArgument(bool value)
			: ValueType(Type::Bool)
			, BooleanValue(value)
		{}
		AttributeArgument(double value)
			: ValueType(Type::Double)
			, DoubleValue(value)
		{}
		AttributeArgument(Type argType, const char* value)
			: ValueType(argType)
			, TextValue(value)
		{
			assert(argType == Type::Token || argType == Type::Compound || argType == Type::String);
		}
		const char* AsString() const
		{
			assert(ValueType == Type::String);
			return TextValue;
		}
		const char* AsToken() const
		{
			assert(ValueType == Type::Token);
			return TextValue;
		}
		const char* AsCompound() const
		{
			assert(ValueType == Type::Compound);
			return TextValue;
		}
		bool AsBool() const
		{
			assert(ValueType == Type::Bool);
			return BooleanValue;
		}
		int64_t AsInteger() const
		{
			assert(ValueType == Type::Int);
			return IntegerValue;
		}
		double AsDouble() const
		{
			assert(ValueType == Type::Double);
			return DoubleValue;
		}

	private:
		Type ValueType;
		union
		{
			const char* TextValue;
			double DoubleValue;
			int64_t IntegerValue;
			bool BooleanValue;
		};
	};

	enum class TypeAttribute;
	enum class PropertyAttribute;
	template<typename AttributeType>
	struct ReflectedAttribute
	{
		ReflectedAttribute()
		{}
		ReflectedAttribute(AttributeType type, std::initializer_list<AttributeArgument> args)
			: Type(type)
		{
			// TODO: ASSERT INITIALIZER LIST SIZE < 4
			for (int i = 0; i < args.size(); i++)
			{
				Args[i] = *(args.begin() + i);
			}
		}
		AttributeType Type;
		std::array<AttributeArgument, 4> Args;
	};

	class ReflectedVariable
	{
	public:
		ReflectedVariable()
			: IsConst(false)
			, IsRef(false)
		{}
		ReflectedVariable(const uint32_t id, const char* name, uint8_t pointerLevels, bool isConst, bool isRef,
			size_t size, size_t offsetToBase,
			std::initializer_list<ReflectedAttribute<PropertyAttribute>> attributes)
			: TypeId(id), Name(name)
			, PointerLevels(pointerLevels), IsConst(isConst), IsRef(isRef)
			, Size(size)
			, OffsetToBase(offsetToBase)
		{
			// TODO: ASSERT INITIALIZER LIST SIZE < 4
			for (int i = 0; i < attributes.size(); i++)
			{
				Attributes[i] = *(attributes.begin() + i);
			}
		}
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
		template<typename T>
		const T* ReadMemoryAs(const void* thisObject) const
		{
			return (T*)((const char*)thisObject + OffsetToBase);
		}
		const char* Name = nullptr;
		const size_t Size = 0;
		const size_t OffsetToBase = 0;
		const TypeIdentifier TypeId = InvalidTypeIdentifier;
		const unsigned char PointerLevels = 0u;
		std::array<ReflectedAttribute<PropertyAttribute>, 4> Attributes;
		// Can't inline initialize bit fields until C++20
		const bool IsConst : 1;
		const bool IsRef : 1;
	};

	class ReflectedType;
	const ReflectedType& TypeOf(TypeIdentifier typeId);

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
		const ReflectedType& GetBaseAtIndex(size_t index) const
		{
			return Vzor::TypeOf(BaseTypes[index]);
		}

		const TypeIdentifier TypeId = InvalidTypeIdentifier;
		const char* Name = nullptr;
		// TODO
		//std::array<ReflectedAttribute<TypeAttribute>, 4> Attributes;
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


	enum class InheritedReflection
	{
		None,
		BaseClassAlreadyReflected
	};

	template<typename T, InheritedReflection ReflectionStatus = InheritedReflection::None>
	struct EnableReflectionFromThis;

	template<typename T>
	struct EnableReflectionFromThis<T, InheritedReflection::None>
		: public RuntimeReflectionInfo
	{
		EnableReflectionFromThis()
		{
			ReflectedTypeId = TypeIdOf<T>();
			// TODO:
			//static_assert(!std::is_base_of<RuntimeReflectionInfo, T>::value,
			//    "If compilation fails here, you have a class whose base is also reflected. "
			//    "Use EnableReflectionFromThis<T, InheritedReflection::BaseClassAlreadyReflected> instead.");
		}
	};

	template<typename T>
	struct EnableReflectionFromThis<T, InheritedReflection::BaseClassAlreadyReflected>
	{
		EnableReflectionFromThis()
		{
			((RuntimeReflectionInfo*)this)->ReflectedTypeId = TypeIdOf<T>();
			static_assert(std::is_base_of<RuntimeReflectionInfo, T>::value,
				"If compilation fails here, you don't have a class whose base is also reflected. "
				"Use EnableReflectionFromThis<T> instead.");
		}
	};

	inline const ReflectedType& TypeOf(const RuntimeReflectionInfo& obj)
	{
		return Detail::AllReflectedTypes[obj.ReflectedTypeId.GetValue()];
	}
};
