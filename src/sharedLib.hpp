#pragma once

#include <string>
#include <filesystem>

class SharedLib {
public:
	SharedLib() noexcept
		: handle(nullptr), errorMessage("") {}
	SharedLib(std::filesystem::path path) noexcept;
	
	SharedLib(const SharedLib&) = default;
	SharedLib(SharedLib&& other) noexcept
		: handle(other.handle), errorMessage(std::move(other.errorMessage)) {
		other.handle = nullptr;
	}

	SharedLib& operator=(const SharedLib&) = default;
	SharedLib& operator=(SharedLib&& other) noexcept {
		if (this != &other) {
			handle = other.handle;
			errorMessage = std::move(other.errorMessage);
			other.handle = nullptr;
		}
		return *this;
	}

	~SharedLib() noexcept;


	void* getFunction(const char* name) const noexcept;
	bool hasFunction(const char* name) const noexcept {
		// TODO: should handle errors and reset them to previous state 
		// and check if the error is about missing function or something else
		return getFunction(name) != nullptr;
	}

	bool isValid() const noexcept {
		return handle != nullptr;
	}

	const std::string& getError() noexcept {
		return errorMessage;
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