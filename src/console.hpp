#pragma once

#include <string>
#include <vector>
#include <mutex>
#include <iostream>
#include <fstream>
#include <chrono>
#include <format>

class Console {
public:

	struct Log {
		enum class Type {
			message,
			warning,
			error,
			debug
		};
		Type type;
		std::string what;
	};

	static void log(Log::Type type, const std::string& what) {
		std::lock_guard<std::mutex> lock(mutex);
		_logs.emplace_back(type, what);
		if (type != Log::Type::message)
			std::cout << (int)type << ": " << what << "\n";
	}

	[[nodiscard]]
	static const std::vector<Log>& getLogs() noexcept {
		return _logs;
	}

	static void createConsoleLogDumpFile(std::string_view why) {
		auto now = std::chrono::system_clock::now();
		auto now_s = std::chrono::floor<std::chrono::seconds>(now);

		std::string filename = "drone_piloting_console_dump_" + 
			std::to_string(std::chrono::system_clock::to_time_t(now)) + ".txt";
		std::ofstream file(filename);
		assert(file && "Failed to create console log dump file");

		file << "Console log dump from drone piloting.\nReason: " << why << "\n\n"
			<< "At: " << std::format("{:%Y.%m.%d, %H:%M:%S}", now_s) << "\n\n"
			<< "This file contains all logs that were logged to the console, and is useful for debugging crashes and other issues. It is recommended to include this file when reporting issues.\n\n"
			<< "Logs:\n";

		for (const Log& log : _logs) {
			file << toString(log.type) << ": " << log.what << "\n";
		}
		file.flush();
		file.close();
	}

private: 

	[[nodiscard]]
	static std::string_view toString(Log::Type type) noexcept {
		switch (type) {
		case Log::Type::message:
			return "Message";
		case Log::Type::warning:
			return "Warning";
		case Log::Type::error:
			return "Error";
		case Log::Type::debug:
			return "Debug";
		default:
			assert(false && "Unknown log type");
			return "Unknown";
		}
	}

	static inline std::vector<Log> _logs;
	static inline std::mutex mutex;
};