#include "App.hpp"

#include <iostream>

#include "benchmark.hpp"
#include "console.hpp"

int main() {
	try {
		createSettings();

		if constexpr (benchmark::enabled) {
			benchmark::createFile("Trondheim map2");

			benchmark::file << "Benchmarking with " << benchmark::runs << " runs\n";

			long long totalStartupTime = 0;
			long long totalShutdownTime = 0;

			for (size_t i = 0; i < benchmark::runs; i++) {
				benchmark::file << "\n\n\nRun " << i + 1 << "/" << benchmark::runs << "\n";

				auto start = std::chrono::high_resolution_clock::now();
				App::startup();
				auto end = std::chrono::high_resolution_clock::now();
				benchmark::file << "Startup time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms\n";
				totalStartupTime += std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

				App::run();

				start = std::chrono::high_resolution_clock::now();
				App::shutdown();
				end = std::chrono::high_resolution_clock::now();
				benchmark::file << "Shutdown time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms\n";
				totalShutdownTime += std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
			}

			benchmark::file << "\n\n\nSummary:\n";
			benchmark::file << "Total startup time: " << totalStartupTime << " ms\n";
			benchmark::file << "Average startup time: " << totalStartupTime / static_cast<double>(benchmark::runs) << " ms\n";
			benchmark::file << "Total shutdown time: " << totalShutdownTime << " ms\n";
			benchmark::file << "Average shutdown time: " << totalShutdownTime / static_cast<double>(benchmark::runs) << " ms\n";

			benchmark::file << "\nAverage frame time: " << benchmark::averageFrameTimeSum / static_cast<double>(benchmark::runs) << " ms\n";
			benchmark::closeFile();
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
}

#ifdef WIN32
#include <windows.h>

int APIENTRY WinMain(HINSTANCE, HINSTANCE, PSTR, int) {
	return main();
}
#endif