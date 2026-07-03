#include "App.hpp"

#include <iostream>

#include "benchmark.hpp"
#include "console.hpp"

#include <chrono>

int main() {
	try {
		if constexpr (benchmark::enabled) {
			benchmark::initialize();
			const auto& config = benchmark::getConfig();

			for (size_t i = 0; i < config.runs; i++) {
				benchmark::beginRun(i);

				auto start = std::chrono::high_resolution_clock::now();
				App::startup();
				auto end = std::chrono::high_resolution_clock::now();
				benchmark::recordStartupTime(
					std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());

				App::run();

				start = std::chrono::high_resolution_clock::now();
				App::shutdown();
				end = std::chrono::high_resolution_clock::now();
				benchmark::recordShutdownTime(
					std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
			}

			benchmark::finalize();
			return 0;
		}
		else {
			App::startup();
			
			App::run();
			
			App::shutdown();
		}
	}
	catch (const std::exception& e) {
		Console::log(Console::Log::Type::error, std::string("Unhandled exception, caused fatal crash. Was: ") + e.what());
		Console::createConsoleLogDumpFile("Application crash");
		std::_Exit(EXIT_FAILURE);
	}
	catch (...) {
		Console::log(Console::Log::Type::error, std::string("Unhandled exception, caused fatal crash. Exception not derived from std::exception."));
		Console::createConsoleLogDumpFile("Application crash");
		std::_Exit(EXIT_FAILURE);
	}
}

#ifdef WIN32
#include <windows.h>

int APIENTRY WinMain(HINSTANCE, HINSTANCE, PSTR, int) {
	return main();
}
#endif
