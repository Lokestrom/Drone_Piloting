#pragma once
#include <string>

#ifdef _WIN32
#include <windows.h>

std::string OpenFileExplorer(HWND owner = nullptr);
#endif // _WIN32