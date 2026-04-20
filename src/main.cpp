#include "App.hpp"

#include <iostream>

int main() {
	
	createSettings();

	App::startup();

	App::run();

	App::shutdown();
}