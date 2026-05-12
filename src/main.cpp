#include "App.hpp"

#include <iostream>

int main() {
	
	createSettings();

	App::startup();

	App::run();

	App::shutdown();
}

#ifdef WIN32
#include <windows.h>

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow) {
	main();
}
#endif