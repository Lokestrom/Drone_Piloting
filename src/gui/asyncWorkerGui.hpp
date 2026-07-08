#pragma once

#include "../structures/asyncWorker.hpp"

#include <filesystem>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

namespace gui {

class AsyncWorkerGui {
public:
	using Path = std::pair<std::string, std::filesystem::path>;

	AsyncWorkerGui(std::vector<Path> paths);

	void setPhase(std::string phase);
	[[nodiscard]]
	std::string phase() const;
	void renderDetails(const AsyncWorker::Status& status) const;
	void renderErrorDetails(const AsyncWorker::Status& status) const;

private:
	std::vector<Path> _paths;
	mutable std::mutex _phaseMutex;
	std::string _phase = "Waiting to start";

	void renderPaths() const;
};

}
