#pragma once

#include "ImGui/imgui.h"

#include <concepts>
#include <type_traits>
#include <memory>
#include <unordered_map>
#include <string>
#include <algorithm>

// TODO: Load from a file and save user preferences
namespace settings {

struct IValue {
	virtual ~IValue() = default;
	virtual void set() = 0;
	virtual const std::string& name() const noexcept = 0;
};

namespace {
template <typename T>
concept AddAssignable = requires(T a, const T b) { a += b; };

template <typename T>
concept SubAssignable = requires(T a, const T b) { a -= b; };

template <typename T>
concept MulAssignable = requires(T a, const T b) { a *= b; };

template <typename T>
concept DivAssignable = requires(T a, const T b) { a /= b; };

template <typename T>
inline static constexpr bool assignNoexcept = requires(T a, const T b) { { a = b } noexcept; };

template <typename T>
inline static constexpr bool addAssignNoexcept = requires(T a, const T b) { { a += b } noexcept; };

template <typename T>
inline static constexpr bool subAssignNoexcept = requires(T a, const T b) { { a -= b } noexcept; };

template <typename T>
inline static constexpr bool mulAssignNoexcept = requires(T a, const T b) { { a *= b } noexcept; };

template <typename T>
inline static constexpr bool divAssignNoexcept = requires(T a, const T b) { { a /= b } noexcept; };
}

template <typename T>
class ValueHandle;


template <typename T>
	requires !std::is_pointer_v<std::remove_reference_t<T>> &&
			!std::is_const_v<std::remove_reference_t<T>> 
class Value : public IValue 
{
	friend class ValueHandle<T>;

	using nonRefrenceT = std::remove_reference_t<T>;

public:
	using setFunctionT = void (*)(const std::string&, T&);

	Value() = delete;
	Value(const std::string& name, const T& defaultValue, setFunctionT setFunction)
		: _name(name)
		, _value(defaultValue)
		, _defaultValue(defaultValue)
		, _setFunction(setFunction)
		, _refCount(0) {}

	Value(const std::string& name, T value, const nonRefrenceT& defaultValue, setFunctionT setFunction)
		requires std::is_reference_v<T>
		: _name(name)
	, _value(value)
	, _defaultValue(defaultValue)
	, _setFunction(setFunction)
	, _refCount(0) 
	{}

	virtual ~Value() {
		assert(_refCount == 0 && "There is still references to this value");
	};

	Value(Value&) = delete;
	Value& operator=(Value&) = delete;
	Value(Value&&) = delete;
	Value& operator=(Value&&) = delete;

	Value& operator=(const nonRefrenceT& value) noexcept(assignNoexcept<nonRefrenceT>) {
		_value = value;
		return *this;
	}

	Value& operator+=(const nonRefrenceT& value) noexcept(addAssignNoexcept<nonRefrenceT>)
		requires AddAssignable<nonRefrenceT>
	{
		_value += value;
		return *this;
	}
	Value& operator-=(const nonRefrenceT& value) noexcept(subAssignNoexcept<nonRefrenceT>)
		requires SubAssignable<nonRefrenceT>
	{
		_value -= value;
		return *this;
	}
	Value& operator*=(const nonRefrenceT& value) noexcept(mulAssignNoexcept<nonRefrenceT>)
		requires MulAssignable<nonRefrenceT>
	{
		_value *= value;
		return *this;
	}
	Value& operator/=(const nonRefrenceT& value) noexcept(divAssignNoexcept<nonRefrenceT>)
		requires DivAssignable<nonRefrenceT>
	{
		_value /= value;
		return *this;
	}

	operator nonRefrenceT&() noexcept { return _value; }

	const std::string& name() const noexcept { return _name; }
	void reset() noexcept(assignNoexcept<nonRefrenceT>) { _value = _defaultValue; }
	ValueHandle<T> getHandle() noexcept {
		return ValueHandle<T>(*this);
	};

	virtual void set() { static_cast<setFunctionT>(_setFunction)(_name, _value); }

protected:
	Value(const std::string& name, const T& defaultValue, void* setFunction)
		: _name(name)
		, _value(defaultValue)
		, _defaultValue(defaultValue)
		, _setFunction(setFunction)
		, _refCount(0) {}

	T _value;
	const void* _setFunction;

private:
	size_t _refCount;
	const std::string _name;
	const std::remove_reference_t<T> _defaultValue;
};

template <typename T>
class ValueHandle {
public:
	ValueHandle(Value<T>& container) noexcept
		: _container(container) {
		_container._refCount++;
	}

	~ValueHandle() noexcept {
		assert(_container._refCount > 0 && "Incorrect reference count to value");
		_container._refCount--;
	}

	ValueHandle(ValueHandle<T>& other) 
		: _container(other._container){
		_container._refCount++;
	}

	ValueHandle(ValueHandle<T>&& other)
		: _container(other._container) {
		_container._refCount++;
	}

	operator T() noexcept {
		assert(_container._refCount > 0 && "Incorrect reference count to value");
		return _container._value;
	}

	T& get() noexcept {
		assert(_container._refCount > 0 && "Incorrect reference count to value");
		return _container._value;
	}

private:
	Value<T>& _container;
};

template <typename T>
	requires std::totally_ordered<T>
class ValueWithRange : public Value<T> {
	using Value<T>::_value;
	using Value<T>::name;
	using Value<T>::_setFunction;

public:
	using setFunctionT = void (*)(const std::string&, T&, const T&, const T&);

	ValueWithRange() = delete;
	ValueWithRange(const std::string& name, const T& defaultValue, setFunctionT setFunction, const T& min, const T& max)
		: Value<T>(name, defaultValue, setFunction)
		, _min(min)
		, _max(max) {
		assert(min < max && "Minimum value must be less than the maximum!");
		assert((defaultValue >= min && defaultValue <= max) && "Default value must lay inside the range!");
	}

	ValueWithRange& operator=(const T& value) noexcept(assignNoexcept<T>) {
		_value = value;
		clamp();
		return *this;
	}

	ValueWithRange& operator+=(const T& value) noexcept(addAssignNoexcept<T>)
		requires AddAssignable<T>
	{
		_value += value;
		clamp();
		return *this;
	}
	ValueWithRange& operator-=(const T& value) noexcept(subAssignNoexcept<T>)
		requires SubAssignable<T>
	{
		_value -= value;
		clamp();
		return *this;
	}
	ValueWithRange& operator*=(const T& value) noexcept(mulAssignNoexcept<T>)
		requires MulAssignable<T>
	{
		_value *= value;
		clamp();
		return *this;
	}
	ValueWithRange& operator/=(const T& value) noexcept(divAssignNoexcept<T>)
		requires DivAssignable<T>
	{
		_value /= value;
		clamp();
		return *this;
	}

	void set() final {
		static_cast<setFunctionT>(_setFunction)(name(), _value, _min, _max);
	}

private:
	void clamp() {
		if (_value < _min)
			_value = _min;
		if (_value > _max)
			_value = _max;
	}

private:
	const T _min;
	const T _max;
};

// all value and subcategory references are invalidated only when the category is destroyed
class SettingsCategory {
public:
	SettingsCategory() = delete;
	SettingsCategory(const std::string& name) noexcept
		: name(name)		  
	{}
	~SettingsCategory() noexcept = default;

	SettingsCategory(const SettingsCategory&) = delete;
	SettingsCategory& operator=(const SettingsCategory&) = delete;

	SettingsCategory(SettingsCategory&&) noexcept = default;
	SettingsCategory& operator=(SettingsCategory&&) noexcept = default;

	template <typename T>
	void add(std::unique_ptr<Value<T>> value) {
		auto find = [value](const std::unique_ptr<IValue>& arrVal) {
			return arrVal->name() == value->name();
		};
		assert(!std::ranges::any_of(values, find) && "Category already contains a value with this name");
		values.push_back(static_cast<std::unique_ptr<IValue>>(std::move(value)));
	}
	template <typename ValueT, typename... Args>
		requires std::derived_from<ValueT, IValue> && std::constructible_from<ValueT, std::string, Args...>
	void emplace(const std::string& valName, Args&&... args) {
		auto find = [valName](const std::unique_ptr<IValue>& arrVal) {
			return arrVal->name() == valName;
		};
		assert(!std::ranges::any_of(values, find) && "Category already contains a value with this name");
		values.emplace_back(std::make_unique<ValueT>(valName, std::forward<Args>(args)...));
	}

	template <typename T>
	Value<T>& get(const std::string& valName) {
		auto find = [valName](const std::unique_ptr<IValue>& arrVal) {
			return arrVal->name() == valName;
		};
		assert(std::ranges::any_of(values, find) && "Category does not contain this value name");
		auto it = std::ranges::find_if(values, find);
		auto ptr = dynamic_cast<Value<T>*>(it->get());
		assert(ptr && "Type mismatch when getting settings value");
		return *ptr;
	}

	SettingsCategory& addSubCategory(const std::string& subName) {
		auto find = [subName](const std::unique_ptr<SettingsCategory>& category) {
			return subName == category->name;
		};
		assert(!std::ranges::any_of(subCategories, find) && "Category already contains a sub category with this name");
		subCategories.emplace_back(std::make_unique<SettingsCategory>(subName));
		return *subCategories.back();
	}
	SettingsCategory& getSubCategory(const std::string& subName) {
		auto find = [subName](const std::unique_ptr<SettingsCategory>& category) {
			return subName == category->name;
		};
		assert(std::ranges::any_of(subCategories, find) && "Category does not contain this sub category name");
		return *std::ranges::find_if(subCategories, find)->get();
	}

	auto& getValues() noexcept {
		return values;
	}
	auto& getSubCategories() noexcept {
		return subCategories;
	}

	const std::string name;
private:
	std::vector<std::unique_ptr<IValue>> values;
	std::vector<std::unique_ptr<SettingsCategory>> subCategories;
};

class Settings {
public:
	SettingsCategory& newCategory(const std::string& name) {
		auto find = [name](const std::unique_ptr<SettingsCategory>& category) {
			return name == category->name;
		};
		assert(!std::ranges::any_of(categories, find) && "Settings already contains that category");
		categories.emplace_back(std::make_unique<SettingsCategory>(name));
		return *categories.back();
	}

	SettingsCategory& get(const std::string& name) {
		auto find = [name](const std::unique_ptr<SettingsCategory>& category) {
			return name == category->name;
		};
		assert(std::ranges::any_of(categories, find) && "Settings don't contain that category");
		return *std::ranges::find_if(categories, find)->get();
	}

	auto begin() noexcept {
		return categories.begin();
	}
	auto end() noexcept {
		return categories.end();
	}

private:
	std::vector<std::unique_ptr<SettingsCategory>> categories;
};
}