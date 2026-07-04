#include "sharedLib.hpp"

#include <assert.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

SharedLib::SharedLib(std::filesystem::path path)
	: handle(nullptr)
	, errorMessage("") {
	assert(path.has_extension() == false && "path should not have an extention since it is handled here");
#ifdef _WIN32
	path += ".dll";
	assert(std::filesystem::is_regular_file(path) && "path is not a file");
	static_assert(sizeof(LPSTR) == sizeof(const char*));
	handle = LoadLibraryA(path.string().c_str());
	if (!handle) {
		DWORD err = GetLastError();
		LPSTR buffer = nullptr;

		size_t size = FormatMessageA(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_SYSTEM |
				FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			err,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPSTR)&buffer,
			0,
			NULL);

		if (size && buffer) {
			const char* msg = buffer;
			errorMessage = msg;
			LocalFree(buffer);
		}
		else {
			errorMessage = "Failed to load library and retrieve error message. But got error code: " + std::to_string(err);
		}

	}
#else
	path += ".so";
	assert(std::filesystem::is_regular_file(path) && "path is not a file");
	dlerror(); // clear
	handle = dlopen(path.string().c_str(), RTLD_NOW);
	if (!handle) {
		errorMessage = dlerror();
	}
#endif
}

SharedLib::SharedLib(SharedLib&& other) noexcept
	: handle(std::exchange(other.handle, nullptr))
	, errorMessage(std::move(other.errorMessage)) {
}

SharedLib& SharedLib::operator=(SharedLib&& other) noexcept {
	if (this == &other) {
		return *this;
	}
	if (handle != nullptr) {
#ifdef _WIN32
		FreeLibrary((HMODULE)handle);
#else
		dlclose(handle);
#endif
	}
	handle = std::exchange(other.handle, nullptr);
	errorMessage = std::move(other.errorMessage);
	return *this;
}

SharedLib::~SharedLib() noexcept {
	if (handle == nullptr)
		return;

#ifdef _WIN32
	FreeLibrary((HMODULE)handle);
#else
	dlclose(handle);
#endif
}

void* SharedLib::getFunction(const char* name) const noexcept {
#ifdef _WIN32
	return GetProcAddress((HMODULE)handle, name);
#else
	return dlsym(handle, name);
#endif
}
