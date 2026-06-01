#include "fileExplorer.hpp"

#ifdef _WIN32
#include <shobjidl.h>
#include <windows.h>

std::string OpenFileExplorer(HWND owner) {
	std::string result;
	IFileDialog* pfd = nullptr;
	HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
	if (!SUCCEEDED(hr))
		return "";
	DWORD options;
	pfd->GetOptions(&options);
	pfd->SetOptions(options | FOS_PICKFOLDERS);
	if (!SUCCEEDED(pfd->Show(owner))) {
		pfd->Release();
		return "";
	}

	IShellItem* psi;
	if (!SUCCEEDED(pfd->GetResult(&psi))) {
		psi->Release();
		return "";
	}

	PWSTR path = nullptr;
	if (!SUCCEEDED(psi->GetDisplayName(SIGDN_FILESYSPATH, &path))) {
		return "";
	}
	char buffer[MAX_PATH];
	wcstombs(buffer, path, MAX_PATH);
	result = buffer;
	CoTaskMemFree(path);

	pfd->Release();
	psi->Release();

	return result;
}
#endif // _WIN32