#include "asyncWorkerGui.hpp"

#include <ImGui/imgui.h>

namespace gui {

AsyncWorkerGui::AsyncWorkerGui(std::vector<Path> paths)
	: _paths(std::move(paths)) {
}

void AsyncWorkerGui::setPhase(std::string phase) {
	const std::lock_guard lock(_phaseMutex);
	_phase = std::move(phase);
}

std::string AsyncWorkerGui::phase() const {
	const std::lock_guard lock(_phaseMutex);
	return _phase;
}

void AsyncWorkerGui::renderDetails(const AsyncWorker::Status&) const {
	const std::string currentPhase = phase();
	ImGui::TextUnformatted(currentPhase.c_str());
	renderPaths();
}

void AsyncWorkerGui::renderErrorDetails(const AsyncWorker::Status&) const {
	renderPaths();
}

void AsyncWorkerGui::renderPaths() const {
	for (const auto& [label, path] : _paths)
		ImGui::TextWrapped("%s: %s", label.c_str(), path.string().c_str());
}

}
