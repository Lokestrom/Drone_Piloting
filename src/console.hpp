#pragma once

#include <string>
#include <vector>

class Console {
public:
	enum class Type {
		meassage,
		warning,
		error,
		debug
	};

	struct Log {
		Type type;
		std::string what;
	};

	static void log(Type type, std::string what) {
		_logs.emplace_back(type, what);
	}

	[[nodiscard]]
	static const std::vector<Log>& getLogs() noexcept {
		return _logs;
	}

private: 
	static inline std::vector<Log> _logs;
};