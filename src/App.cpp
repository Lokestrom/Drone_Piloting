#include "App.hpp"

#include "windows/windowSetup.hpp"
#include "console.hpp"
#include "rendering/helpers.hpp"
#include "benchmark.hpp"
#include "gui/settingsGui.hpp"
#include "gui/asyncWorkerGui.hpp"
#include "SettingNames.hpp"

#include <memory>
#include <utility>

#include <vulkan/vk_enum_string_helper.h>

static void glfwErrorCallback(int error, const char* description) {
	Console::log(Console::Log::Type::error, std::string("GLFW error: Code: ") + std::to_string(error) + ", What: " + description);
}

static void check_vk_result(VkResult err) {
	if (err == VK_SUCCESS) [[likely]]
		return;
	__debugbreak();
	Console::log(Console::Log::Type::error, std::string("Vulkan error: ") + string_VkResult(err));
	if (err < 0)
		std::terminate();
}

void App::startup() {
	createSettings();

	glfwSetErrorCallback(glfwErrorCallback);
	if (!glfwInit())
		throw std::runtime_error("GLFW initialization failed");

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	window = glfwCreateWindow(1280, 720, "Drone piloting", nullptr, nullptr);
	if (!glfwVulkanSupported())
		throw std::runtime_error("No vulkan support");

	std::vector<const char*> extensions;
	uint32_t glfwExtensionsCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionsCount);
	for (uint32_t i = 0; i < glfwExtensionsCount; i++)
		extensions.push_back(glfwExtensions[i]);
	vulkan::App::startup(extensions);

	VkSurfaceKHR surface;
	VkResult err = glfwCreateWindowSurface(static_cast<VkInstance>(*vulkan::App::instance), window, nullptr, &surface);
	check_vk_result(err);

	glfwGetFramebufferSize(window, &width, &height);
	wd = &vulkan::App::mainWindowData;
	vulkan::App::startupWindow(wd, static_cast<vk::SurfaceKHR>(surface), width, height);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	(void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;	  // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;	  // Enable Multi-Viewport / Platform Windows
	io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
	// io.ConfigViewportsNoAutoMerge = true;
	// io.ConfigViewportsNoTaskBarIcon = true;

	ImGui::StyleColorsDark();

	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	ImGui_ImplGlfw_InitForVulkan(window, true);
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = static_cast<VkInstance>(*vulkan::App::instance);
	init_info.PhysicalDevice = static_cast<VkPhysicalDevice>(*vulkan::App::physicalDevice);
	init_info.Device = static_cast<VkDevice>(*vulkan::App::device);
	init_info.QueueFamily = vulkan::App::queueFamily;
	init_info.Queue = static_cast<VkQueue>(*vulkan::App::queue);
	init_info.PipelineCache = static_cast<VkPipelineCache>(*vulkan::App::pipelineCache);
	init_info.DescriptorPool = static_cast<VkDescriptorPool>(*vulkan::App::descriptorPool);
	init_info.RenderPass = wd->RenderPass;
	init_info.Subpass = 0;
	init_info.MinImageCount = vulkan::App::minImageCount;
	init_info.ImageCount = wd->ImageCount;
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	init_info.Allocator = nullptr;
	init_info.CheckVkResultFn = check_vk_result;
	ImGui_ImplVulkan_Init(&init_info);

	addPlayer("Default");
	auto& player = getCurrentPlayer();

	std::filesystem::path dronePath = ASSET_DIR "Drones/testDrone";
	std::filesystem::path mapPath = ASSET_DIR "Maps/TestMap";
	if constexpr (benchmark::enabled) {
		const auto& config = benchmark::getConfig();
		dronePath = config.drone;
		mapPath = config.map;
	}

	if constexpr (benchmark::enabled) {
		Drone loadedDrone;
		if (!loadedDrone.load(dronePath)) {
			throw std::runtime_error("Failed to load drone: " + dronePath.string());
		}
		player.replaceDrone(std::move(loadedDrone));
		const auto& position = benchmark::getConfig().dronePosition;
		player.getDrone()->getPosition() = glm::vec3(position[0], position[1], position[2]);
		map.emplace();
		if (!map->load(mapPath)) {
			map.reset();
			throw std::runtime_error("Failed to load map: " + mapPath.string());
		}
		loadedMapPath = mapPath;
	}

	gui::App::startup();
	setupWindows();

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

	if constexpr (!benchmark::enabled) {
		startInitialAsyncLoad(std::move(dronePath), std::move(mapPath));
	}
}

void App::shutdown() {

	try {
		vulkan::App::waitIdle();
	}
	catch (...) {
		throw std::runtime_error("Failed to wait when shutting down");
	}
	AsyncWorker::shutdown();

	map.reset();
	for (auto& player : players)
		player->releaseDrone();

	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	gui::App::shutdown();
	vulkan::App::shutdown();

	glfwDestroyWindow(window);
	glfwTerminate();
}

#include "Input/InputEventHandler.hpp"

void App::updateMouseInput() {
	glm::vec<2, double> mousePos;
	glfwGetCursorPos(window, &mousePos.x, &mousePos.y);


	if (!gui::App::enabled) {
		glfwSetCursorPos(window, (double)width / 2.0, (double)height / 2.0);
		InputEventHandler::mouseDelta = mousePos - glm::vec<2, double>{ (double)width / 2.0, (double)height / 2.0 };
		if (std::abs(InputEventHandler::mouseDelta.x) < 1)
			InputEventHandler::mouseDelta.x = 0;
		if (std::abs(InputEventHandler::mouseDelta.y) < 1)
			InputEventHandler::mouseDelta.y = 0;
	}
	else {
		InputEventHandler::mouseDelta = InputEventHandler::lastMousePos - mousePos;
		InputEventHandler::lastMousePos = mousePos;
	}
}

void App::startInitialAsyncLoad(
	std::filesystem::path dronePath,
	std::filesystem::path mapPath) {
	auto workerGui = std::make_shared<gui::AsyncWorkerGui>(
		std::vector<gui::AsyncWorkerGui::Path>{
			{ "Drone", dronePath },
			{ "Map", mapPath } });
	const std::string playerName = getCurrentPlayer().name();

	AsyncWorker::submit(AsyncWorker::WorkOrder{
		.description = "initial map and drone",
		.work = [dronePath = std::move(dronePath),
					mapPath = std::move(mapPath),
					playerName,
					workerGui]() mutable -> AsyncWorker::CompletionFn {
			vulkan::App::waitIdle();

			workerGui->setPhase("Loading drone");
			Drone loadedDrone;
			if (!loadedDrone.load(dronePath))
				throw std::runtime_error("The default drone failed to load.");

			workerGui->setPhase("Loading map");
			Map loadedMap;
			if (!loadedMap.load(mapPath))
				throw std::runtime_error("The default map failed to load.");

			workerGui->setPhase("Installing loaded assets");
			return AsyncWorker::CompletionFn{
				[playerName,
					mapPath = std::move(mapPath),
					loadedDrone = std::move(loadedDrone),
					loadedMap = std::move(loadedMap)]() mutable {
					const auto player = std::ranges::find_if(
						players,
						[&](const std::unique_ptr<Player>& candidate) {
							return candidate->name() == playerName;
						});
					if (player == players.end())
						throw std::runtime_error(
							"The player for the initial drone no longer exists.");

					player->get()->replaceDrone(std::move(loadedDrone));
					installMap(std::move(loadedMap), std::move(mapPath));
				}
			};
		},
		.detailsGui = [workerGui](const AsyncWorker::Status& status) {
			workerGui->renderDetails(status);
		},
		.errorDetailsGui = [workerGui](const AsyncWorker::Status& status) {
			workerGui->renderErrorDetails(status);
		} });
}

void App::installMap(
	Map&& replacement,
	std::filesystem::path loadedPath) noexcept {
	map.emplace(std::move(replacement));
	loadedMapPath = std::move(loadedPath);
}

void App::run() {
	if constexpr (benchmark::enabled) {
		const auto& config = benchmark::getConfig();
		// Warmup
		auto start = std::chrono::high_resolution_clock::now();

		while (std::chrono::high_resolution_clock::now() - start <
			   std::chrono::seconds(config.warmupTimeSeconds)) [[likely]] {
			loop();
		}

		// Benchmark
		size_t frames = 0;
		start = std::chrono::high_resolution_clock::now();
		while (std::chrono::high_resolution_clock::now() - start <
			   std::chrono::seconds(config.benchmarkTimeSeconds)) [[likely]] {
			loop();
			frames++;
		}
		auto end = std::chrono::high_resolution_clock::now();

		benchmark::recordAverageFrameTime(
			std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() /
			static_cast<double>(frames));
	}
	else {
		while (!glfwWindowShouldClose(window)) [[likely]] {
			loop();
		}
	}
}

void App::loop() {
	static auto toggleOverlay = settings.get(settingNames::categories::keyBindings)
									.getSubCategory(settingNames::interfaceKeys::subCategory)
									.get<ImGuiKey>(settingNames::interfaceKeys::toggleOverlay)
									.getHandle();
	static auto toggleMenu = settings.get(settingNames::categories::keyBindings)
								 .getSubCategory(settingNames::interfaceKeys::subCategory)
								 .get<ImGuiKey>(settingNames::interfaceKeys::toggleMenu)
								 .getHandle();
	static auto maximumDeltaTime = settings.get(settingNames::categories::simulation)
									   .get<double>(settingNames::simulation::maximumDeltaTime)
									   .getHandle();
	static auto repeatedErrorLimit = settings.get(settingNames::categories::safety)
										 .get<int>(settingNames::safety::repeatedLoopErrorLimit)
										 .getHandle();

	static size_t repeatedErrorCount = 0;

	try {
		glfwPollEvents();
		renderVectors.clear();
		renderPoints.clear();

		// TODO: in overlay holding right click should lock cursor pos and rotate cam.
		// TODO: this should be the gui::App's responsibility
		if (ImGui::IsKeyPressed(toggleOverlay.get(), false)) [[unlikely]] {
			if (!gui::App::enabled)
				gui::App::enabled = true;
			else if (!gui::App::inMenu)
				gui::App::enabled = false;
			gui::App::inMenu = false;
			if (gui::App::enabled) {
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			}
			else {
				glfwSetCursorPos(window, (double)width / 2.0, (double)height / 2.0);
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
			}
		}
		if (ImGui::IsKeyPressed(toggleMenu.get(), false)) [[unlikely]] {
			if (!gui::App::enabled)
				gui::App::enabled = true;
			else if (gui::App::inMenu)
				gui::App::enabled = false;
			gui::App::inMenu = true;
			if (gui::App::enabled) {
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			}
			else {
				glfwSetCursorPos(window, (double)width / 2.0, (double)height / 2.0);
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
			}
		}

		updateMouseInput();

		glfwGetFramebufferSize(window, &width, &height);
		if (width > 0 && height > 0 && (vulkan::App::swapChainRebuild || vulkan::App::mainWindowData.Width != width || vulkan::App::mainWindowData.Height != height)) [[unlikely]] {
			vulkan::App::resizeMainWindow(width, height);
		}
		if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0) [[unlikely]] {
			ImGui_ImplGlfw_Sleep(10);
			return;
		}

		dt = ImGui::GetIO().DeltaTime;
		AsyncWorker::update();

		bool updateCamera = true;
		if (gui::App::enabled && !gui::App::inMenu) {
			updateCamera = false;
		}

		// Hot path is the physics update
		if (!hasActiveWorker()) [[likely]] {
			if (dt < maximumDeltaTime.get()) [[likely]] {
				if (!gui::App::enabled || !gui::App::inMenu) [[likely]]
					for (auto& player : players)
						player->update(player.get() == &getCurrentPlayer(), updateCamera);
			}
			else [[unlikely]] {
				Console::log(Console::Log::Type::warning, "Skiped physics update, delta time to large. "
														  "This message is normal if it happens once after loading a map, drone or focusing the window. "
														  "If this warning persists the drone or map may be to performance intensive for your computer.");
			}
		}

		render();

		InputEventHandler::reset();
		repeatedErrorCount = 0;
	}
	catch (std::exception& e) {
		Console::log(Console::Log::Type::error, std::string("Unhandled exception in loop") + e.what());
		if (repeatedErrorCount++ > static_cast<size_t>(repeatedErrorLimit.get())) {
			throw std::runtime_error("Too many repeated unhandled exceptions in loop");
		}
	}
}

void App::render() {
	gui::App::generateWindows();

	ImDrawData* main_draw_data = ImGui::GetDrawData();
	const bool mainMinimized = (main_draw_data->DisplaySize.x <= 0.0f || main_draw_data->DisplaySize.y <= 0.0f);
	if (!mainMinimized) [[likely]] {
		vulkan::App::beginFrame(wd);

		const bool workerActive = hasActiveWorker();
		vulkan::UniformBufferObject UBO{};
		if (!workerActive) [[likely]]
			UBO = getUBO();
		vulkan::App::render(UBO, !workerActive);
		gui::App::render(wd);

		vulkan::App::endMainFrame(wd);
	}

	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		vulkan::App::updatePlatformWindows();
	}
	if (!mainMinimized) [[likely]] {
		vulkan::App::endFrame(wd);
	}
}

void App::createSettings() {
	auto& keyBindings = settings.newCategory(settingNames::categories::keyBindings);
	auto& interfaceKeyBindings = keyBindings.addSubCategory(settingNames::interfaceKeys::subCategory);
	using KeyValue = settings::Value<ImGuiKey>;
	interfaceKeyBindings.emplace<KeyValue>(settingNames::interfaceKeys::toggleOverlay,
		ImGuiKey_Tab, KeyValue::setFunctionT(gui::keyBindButton));
	interfaceKeyBindings.emplace<KeyValue>(settingNames::interfaceKeys::toggleMenu,
		ImGuiKey_Escape, KeyValue::setFunctionT(gui::keyBindButton));

	auto& safetySettings = settings.newCategory(settingNames::categories::safety);
	safetySettings.emplace<settings::ValueWithRange<int>>(settingNames::safety::repeatedLoopErrorLimit, 10,
		settings::ValueWithRange<int>::setFunctionT(gui::slider), 0, 100,
		"Controls how many consecutive unhandled loop errors are tolerated before the application stops.");

	auto& simulationSettings = settings.newCategory(settingNames::categories::simulation);
	simulationSettings.emplace<settings::ValueWithRange<double>>(settingNames::simulation::maximumDeltaTime, 0.5,
		settings::ValueWithRange<double>::setFunctionT(gui::slider), 0.01, 2.0,
		"Physics updates are skipped when the frame delta time exceeds this value in seconds.");

	auto& performanceSettings = settings.newCategory(settingNames::categories::performance);
	performanceSettings.emplace<settings::ValueWithRange<int>>(settingNames::performance::mapLoadingThreads, 8,
		settings::ValueWithRange<int>::setFunctionT(gui::slider), 1, 32,
		"Number of worker threads used the next time a map is loaded.");

	vulkan::createRenderingSettings();
	createConsoleSettings();
	createWindowsSettings();

	auto& playerSettings = settings.newCategory(settingNames::categories::player);
	playerSettings.emplace<settings::Value<bool>>(settingNames::player::swapOnCreation,
		true, settings::Value<bool>::setFunctionT(gui::checkbox),
		"Automatically selects a player immediately after it is created.");
}


void App::swapToPlayer(const std::string& name) noexcept {
	auto playerNameCheck = [&name](const std::unique_ptr<Player>& player) noexcept {
		return name == player->name();
	};
	assert(name != getCurrentPlayer().name() && "Can't swap to the current selected player, check for multiple players with this name");
	assert(std::ranges::count_if(players, playerNameCheck) == 1 && "There is no or multiple player with the name");
	currentPlayer = std::ranges::find_if(players, playerNameCheck) - players.begin();
}

void App::addPlayer(const std::string& name) {
	players.push_back(std::make_unique<Player>(name));
	if (settings.get(settingNames::categories::player)
			.get<bool>(settingNames::player::swapOnCreation))
		currentPlayer = players.size() - 1;
}

void App::removePlayer(const std::string& name) noexcept {
	auto playerNameCheck = [&name](const std::unique_ptr<Player>& player) noexcept {
		return name == player->name();
	};

	assert(std::ranges::count_if(players, playerNameCheck) == 1 && "There is no or multiple player with the name");

	size_t player = std::ranges::find_if(players, playerNameCheck) - players.begin();
	players.erase(players.begin() + player);

	if (currentPlayer == 0)
		return;
	if (currentPlayer >= player)
		currentPlayer--;
	assert((currentPlayer < players.size() || players.empty()) && "The current player index is out of range");
}

bool App::hasPlayer(const std::string& name) noexcept {
	auto playerNameCheck = [&name](const std::unique_ptr<Player>& player) noexcept {
		return name == player->name();
	};

	return std::ranges::any_of(players, playerNameCheck);
}
