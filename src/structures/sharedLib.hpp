#pragma once

#include <string>
#include <filesystem>

class SharedLib {
public:
	SharedLib() noexcept
		: handle(nullptr), errorMessage("") {}
	SharedLib(std::filesystem::path path) noexcept;
	
	SharedLib(const SharedLib&) = delete;
	SharedLib& operator=(const SharedLib&) = delete;

	SharedLib(SharedLib&& other) noexcept;
	SharedLib& operator=(SharedLib&& other) noexcept;

	~SharedLib() noexcept;

	[[nodiscard]]
	void* getFunction(const char* name) const noexcept;
	[[nodiscard]]
	bool hasFunction(const char* name) const noexcept {
		// TODO: should handle errors and reset them to previous state 
		// and check if the error is about missing function or something else
		return getFunction(name) != nullptr;
	}

	[[nodiscard]]
	bool isValid() const noexcept {
		return handle != nullptr;
	}

	void clearError() noexcept {
		errorMessage.clear();
	}
	[[nodiscard]]
	const std::string& getError() const noexcept {
		return errorMessage;
	}
	[[nodiscard]]
	bool hasError() const noexcept {
		return !errorMessage.empty();
	}

private:
	void* handle;
	// TODO: create a list insted
	std::string errorMessage;
};

//can use cpp 26 reflection to create a shared lib wrapper with functions:
/*
template<typename ...Funcs>
class SharedLibWrapper;

example usage:
SharedLibWrapper<UpdateFn, InfoFn> plugin("path/to/so", "update", "getInfo");

that then creates:
class SharedLibWrapper<UpdateFn, InfoFn> {
public:
	UpdateFn update;
	InfoFn getInfo;
	SharedLib lib;
}
*/