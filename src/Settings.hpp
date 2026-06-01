#pragma once

#include "ImGui/imgui.h"

#include <concepts>
#include <memory>
#include <unordered_map>
#include <string>

namespace settings {

struct IValue {
	virtual ~IValue() = default;
	virtual void set() = 0;
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
class Value : public IValue {
	friend class ValueHandle<T>;

public:
	using setFunctionT = void (*)(const std::string&, T&);

	Value() = delete;
	Value(const std::string& name, const T& defaultValue, setFunctionT setFunction)
		: _name(name)
		, _value(defaultValue)
		, _defaultValue(defaultValue)
		, _setFunction(setFunction)
		, _refCount(0) {}

	virtual ~Value() {
		assert(_refCount == 0 && "There is still references to this value");
	};

	Value(Value&) = delete;
	Value& operator=(Value&) = delete;
	Value(Value&&) = delete;
	Value& operator=(Value&&) = delete;

	Value& operator=(const T& value) noexcept(assignNoexcept) {
		_value = value;
		return *this;
	}

	Value& operator+=(const T& value) noexcept(addAssignNoexcept<T>)
		requires AddAssignable<T>
	{
		_value += value;
		return *this;
	}
	Value& operator-=(const T& value) noexcept(subAssignNoexcept<T>)
		requires SubAssignable<T>
	{
		_value -= value;
		return *this;
	}
	Value& operator*=(const T& value) noexcept(mulAssignNoexcept<T>)
		requires MulAssignable<T>
	{
		_value *= value;
		return *this;
	}
	Value& operator/=(const T& value) noexcept(divAssignNoexcept<T>)
		requires DivAssignable<T>
	{
		_value /= value;
		return *this;
	}

	operator T&() noexcept { return _value; }

	const std::string& name() const noexcept { return _name; }
	void reset() noexcept(assignNoexcept) { _value = _defaultValue; }
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
	const T _defaultValue;
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
	ValueWithRange(const std::string& name, T defaultValue, setFunctionT setFunction, T min, T max)
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
	T _min;
	T _max;
};

class SettingsCategory {
public:
	SettingsCategory() noexcept = default;
	~SettingsCategory() = default;

	SettingsCategory(const SettingsCategory&) = delete;
	SettingsCategory& operator=(const SettingsCategory&) = delete;

	SettingsCategory(SettingsCategory&&) noexcept = default;
	SettingsCategory& operator=(SettingsCategory&&) noexcept = default;

	template <typename T>
	void add(std::unique_ptr<Value<T>> value) {
		assert(!values.contains(value->name()) && "Category already contains a value with this name");
		values[value->name()] = static_cast<std::unique_ptr<IValue>>(std::move(value));
	}
	template <typename T, typename... Args>
		requires std::derived_from<T, IValue> && std::constructible_from<T, std::string, Args...>
	void emplace(const std::string& name, Args&&... args) {
		assert(!values.contains(name) && "Category already contains a value with this name");
		values.emplace(name, std::make_unique<T>(name, std::forward<Args>(args)...));
	}

	template <typename T>
	Value<T>& get(const std::string& name) {
		assert(values.contains(name) && "Category does not contain this value name");
		auto ptr = dynamic_cast<Value<T>*>(values[name].get());
		assert(ptr && "Type mismatch when getting settings value");
		return *ptr;
	}

	auto begin() noexcept {
		return values.begin();
	}
	auto end() noexcept {
		return values.end();
	}

private:
	// can make vector for faster iteration, will lose on lookup but has handles so should not matter
	std::unordered_map<std::string, std::unique_ptr<IValue>> values;
};

class Settings {
public:
	static SettingsCategory& newCategory(const std::string& name) {
		assert(!categorys.contains(name) && "Settings already contains that category");
		categorys[name] = SettingsCategory();
		return categorys[name];
	}

	static SettingsCategory& get(const std::string& category) {
		assert(categorys.contains(category) && "Settings don't contain that category");
		return categorys[category];
	}

	static auto begin() noexcept {
		return categorys.begin();
	}
	static auto end() noexcept {
		return categorys.end();
	}

private:
	static inline std::unordered_map<std::string, SettingsCategory> categorys;
};

static inline ImGuiKey moveForward = ImGuiKey::ImGuiKey_W;
static inline ImGuiKey moveBackwards = ImGuiKey::ImGuiKey_S;
static inline ImGuiKey moveLeft = ImGuiKey::ImGuiKey_A;
static inline ImGuiKey moveRight = ImGuiKey::ImGuiKey_D;
static inline ImGuiKey moveUp = ImGuiKey::ImGuiKey_Space;
static inline ImGuiKey moveDown = ImGuiKey::ImGuiKey_LeftShift;
static inline ImGuiKey rotateLeft = ImGuiKey::ImGuiKey_LeftArrow;
static inline ImGuiKey rotateRight = ImGuiKey::ImGuiKey_RightArrow;
static inline ImGuiKey rotateUp = ImGuiKey::ImGuiKey_UpArrow;
static inline ImGuiKey rotateDown = ImGuiKey::ImGuiKey_DownArrow;
static inline ImGuiKey rollLeft = ImGuiKey::ImGuiKey_Q;
static inline ImGuiKey rollRight = ImGuiKey::ImGuiKey_E;

namespace camera {
static inline double mouseSensitivity = 20.0;
static inline double minRadius = 0.5;
static inline double maxRadius = 1000;
static inline double zoomSpeed = 0.5;
}
}