#pragma once

#include <string>
#include <vector>
#include <mutex>
#include <iostream>

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

	static void log(Log::Type type, std::string what) {
		std::lock_guard<std::mutex> lock(mutex);
		_logs.emplace_back(type, what);
		if (type != Log::Type::message)
			std::cout << (int)type << ": " << what << "\n";
	}

	[[nodiscard]]
	static const std::vector<Log>& getLogs() noexcept {
		return _logs;
	}

private: 
	static inline std::vector<Log> _logs;
	static inline std::mutex mutex;
};