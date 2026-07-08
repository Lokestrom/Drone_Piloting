#pragma once

#include <chrono>
#include <exception>
#include <future>
#include <functional>
#include <optional>
#include <string>

class AsyncWorker {
public:
	struct Status {
		const std::string& description;
		double elapsedSeconds;
		std::exception_ptr exception;
	};

	using CompletionFn = std::move_only_function<void()>;
	using WorkFn = std::move_only_function<CompletionFn()>;
	using GuiSectionFn = std::move_only_function<void(const Status&)>;

	struct WorkOrder {
		std::string description;
		WorkFn work;
		GuiSectionFn detailsGui;
		GuiSectionFn errorDetailsGui;
	};

	AsyncWorker() = delete;

	static void submit(WorkOrder workOrder);
	static void shutdown() noexcept;
	static void update() noexcept;

	[[nodiscard]]
	static bool hasWork() noexcept;
	[[nodiscard]]
	static Status status();
	static void renderGuiSection(const Status& status);
	static void renderErrorGuiSection(const Status& status);
	static void dismissError() noexcept;

	[[nodiscard]]
	static std::string exceptionMessage(std::exception_ptr exception);

private:
	struct ActiveWork {
		explicit ActiveWork(WorkOrder workOrder) noexcept;

		std::string description;
		GuiSectionFn detailsGui;
		GuiSectionFn errorDetailsGui;
		std::future<CompletionFn> future;
		std::chrono::steady_clock::time_point started =
			std::chrono::steady_clock::now();
		std::exception_ptr exception;
	};

	static void reportError(std::exception_ptr exception) noexcept;

	static inline std::optional<ActiveWork> _active;
};
