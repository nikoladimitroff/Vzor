#pragma once

// Taken from https://gist.github.com/loskutov/8c10e34845c71db5177947a0d15ec55f
template <class T, typename = typename std::enable_if<std::is_trivially_copyable<T>::value>::type>
class [[reflect::type]] CustomVector
{
	[[reflect::data]]
	std::size_t capacity_;
	[[reflect::data]]
	std::size_t size_;
	T *data_ = nullptr;

	void ensureCapacity(std::size_t desiredCapacity)
	{
		if (desiredCapacity <= capacity_)
			return;
		capacity_ = std::max(capacity_ * 2, desiredCapacity);
		T *newData = static_cast<T*>(reallocarray(data_, capacity_, sizeof(T)));
		data_ = newData;
	}

public:
	CustomVector(const CustomVector<T> &other) = delete;
	CustomVector<T> operator=(const CustomVector<T> &other) = delete;
	CustomVector(CustomVector<T> &&other)
	{
		std::swap(capacity_, other.capacity_);
		std::swap(size_, other.size_);
		std::swap(data_, other.data_);
	}

	CustomVector(std::size_t n = 0)
		: capacity_{ std::max(size_t(1), n) } // default capacity is 1
		, size_{ n }
		, data_{ static_cast<T*>(malloc(sizeof(T) * capacity_))}
	{}

	T *begin() const
	{
		return data_;
	}

	T *end() const
	{
		return data_ + size_;
	}

	void pushBack(T elem)
	{
		ensureCapacity(size_ + 1);
		data_[size_++] = elem;
	}

	T popBack()
	{
		return data_[--size_];
	}

	T& operator[](size_t ind) const
	{
		return data_[ind];
	}

	T& at(size_t ind) const
	{
		return (ind < size_) ? data_[ind] : throw std::out_of_range("Vector::at");
	}

	size_t capacity() const
	{
		return capacity_;
	}

	~CustomVector() noexcept
	{
		free(data_);
	}

};

#include "vzorgenerated/CustomContainers.h"
