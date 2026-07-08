#include "asyncWorker.hpp"

#include <cassert>
#include <stdexcept>
#include <string>
#include <utility>

#include "../console.hpp"

AsyncWorker::ActiveWork::ActiveWork(WorkOrder workOrder) noexcept
	: description(std::move(workOrder.description))
	, detailsGui(std::move(workOrder.detailsGui))
	, errorDetailsGui(std::move(workOrder.errorDetailsGui)) {
}

void AsyncWorker::submit(WorkOrder workOrder) {
	assert(!_active && "Cannot submit asynchronous work while another operation is active");
	assert(workOrder.work && "An asynchronous work order requires a work function");

	WorkFn work = std::move(workOrder.work);
	_active.emplace(std::move(workOrder));

	try {
		_active->future = std::async(
			std::launch::async,
			[work = std::move(work)]() mutable {
				return work();
			});
	}
	catch (...) {
		reportError(std::current_exception());
	}
}

void AsyncWorker::shutdown() noexcept {
	if (!_active)
		return;

	try {
		if (_active->future.valid())
			_active->future.wait();
	}
	catch (...) { }
	_active.reset();
}

bool AsyncWorker::hasWork() noexcept {
	return _active.has_value();
}

AsyncWorker::Status AsyncWorker::status() {
	assert(_active && "Cannot get async work status while idle");

	return Status{
		.description = _active->description,
		.elapsedSeconds = std::chrono::duration<double>(
			std::chrono::steady_clock::now() - _active->started).count(),
		.exception = _active->exception
	};
}

void AsyncWorker::renderGuiSection(const Status& status) {
	assert(_active && "Cannot render async work GUI while idle");
	if (!_active->detailsGui)
		return;
	try {
		_active->detailsGui(status);
	}
	catch (...) {
		Console::log(
			Console::Log::Type::error,
			"Async worker details GUI failed: " +
				exceptionMessage(std::current_exception()));
	}
}

void AsyncWorker::renderErrorGuiSection(const Status& status) {
	assert(_active && "Cannot render async work error GUI while idle");
	if (!_active->errorDetailsGui)
		return;
	try {
		_active->errorDetailsGui(status);
	}
	catch (...) {
		Console::log(
			Console::Log::Type::error,
			"Async worker error details GUI failed: " +
				exceptionMessage(std::current_exception()));
	}
}

void AsyncWorker::dismissError() noexcept {
	assert(_active && "Cannot dismiss async work error while idle");
	assert(_active->exception && "Cannot dismiss async work error while there is no error");
	_active.reset();
}

void AsyncWorker::update() noexcept {
	if (!_active) [[likely]]
		return;
	if (_active->exception)
		return;
	assert(_active->future.valid() && "The future for the active work is not valid");

	try {
		if (_active->future.wait_for(std::chrono::seconds(0)) !=
			std::future_status::ready) {
			return;
		}

		CompletionFn completion = _active->future.get();
		if (completion)
			completion();
	}
	catch (...) {
		reportError(std::current_exception());
		return;
	}

	_active.reset();
}

std::string AsyncWorker::exceptionMessage(std::exception_ptr exception) {
	assert(exception && "Cannot get async work exception message while there is no exception");
	try {
		std::rethrow_exception(exception);
	}
	catch (const std::exception& e) {
		return e.what();
	}
	catch (...) {
		return "Unknown exception";
	}
}

void AsyncWorker::reportError(std::exception_ptr exception) noexcept {
	assert(_active && "Cannot report an async work error while idle");
	assert(exception && "Cannot report an async work error while there is no exception");
	_active->exception = exception;

	Console::log(Console::Log::Type::error, "Async work failed: " + exceptionMessage(exception));
}
