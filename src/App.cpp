#include "App.hpp"

#include "windows/infoWindow.hpp"
#include "windows/DroneSelect.hpp"
#include "windows/settingsWindow.hpp"

void createSettings() {
	vulkan::createRenderingSettings();
}

static void glfw_error_callback(int error, const char* description) {
	fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

static void check_vk_result(VkResult err) {
	if (err == VK_SUCCESS)
		return;
	__debugbreak();
	fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
	if (err < 0)
		abort();
}
static void check_vk_result(vk::Result err) {
	if (err == vk::Result::eSuccess)
		return;
	__debugbreak();
	fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
	if (err < vk::Result::eSuccess)
		abort();
}

void App::startup() {
	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit())
		throw std::runtime_error("GLFW initialization failed");

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	window = glfwCreateWindow(1280, 720, "Drone piloting", nullptr, nullptr);
	if (!glfwVulkanSupported())
		throw std::runtime_error("No vulkan support");

	ImVector<const char*> extensions;
	uint32_t extensions_count = 0;
	const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&extensions_count);
	for (uint32_t i = 0; i < extensions_count; i++)
		extensions.push_back(glfw_extensions[i]);
	vulkan::App::startup(extensions);

	VkSurfaceKHR surface;
	VkResult err = glfwCreateWindowSurface(vulkan::App::instance, window, nullptr, &surface);
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
	init_info.Instance = vulkan::App::instance;
	init_info.PhysicalDevice = vulkan::App::physicalDevice;
	init_info.Device = vulkan::App::device;
	init_info.QueueFamily = vulkan::App::queueFamily;
	init_info.Queue = vulkan::App::queue;
	init_info.PipelineCache = (VkPipelineCache)vulkan::App::pipelineCache;
	init_info.DescriptorPool = (VkDescriptorPool)vulkan::App::descriptorPool;
	init_info.RenderPass = wd->RenderPass;
	init_info.Subpass = 0;
	init_info.MinImageCount = vulkan::App::minImageCount;
	init_info.ImageCount = wd->ImageCount;
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	init_info.Allocator = nullptr;
	init_info.CheckVkResultFn = check_vk_result;
	ImGui_ImplVulkan_Init(&init_info);

	player = std::make_unique<Player>();
	player->SwapDrone(ASSET_DIR "drones/testDrone");
	map.load(ASSET_DIR "Maps/TestMap");
	vectorScale = glm::vec2(0.3, 0.1);
	gui::App::startup();
	gui::App::addWindow<InfoWindow>();
	gui::App::addWindow<DroneSelectWindow>();
	gui::App::addWindow<SettingsWindow>();

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
}

void App::shutdown() {
	try {
		vulkan::App::device.waitIdle();
	}
	catch (std::exception& e) {
		throw std::runtime_error("Failed to wait when shutting down");
	}

	map.unload();
	player->releaseDrone();

	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	assert(vulkan::GameObjectContainer::getObjects().size() == 0 
		&& "Must destroy all game objects before shutting down vulkan");

	vulkan::App::shutdown();

	glfwDestroyWindow(window);
	glfwTerminate();
}

#include "Input/InputEventHandler.hpp"

void App::run() {
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	ImGuiIO& io = ImGui::GetIO();

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		renderVectors.clear();

		if (ImGui::IsKeyPressed(ImGuiKey_Escape, false)) {
			gui::App::enabled = !gui::App::enabled;
			if (gui::App::enabled) {
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			}
			else {
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
			}
		}

		glfwGetFramebufferSize(window, &width, &height);
		if (width > 0 && height > 0 && (vulkan::App::swapChainRebuild || vulkan::App::mainWindowData.Width != width || vulkan::App::mainWindowData.Height != height)) {
			ImGui_ImplVulkan_SetMinImageCount(vulkan::App::minImageCount);
			ImGui_ImplVulkanH_CreateOrResizeWindow(vulkan::App::instance, vulkan::App::physicalDevice, vulkan::App::device,
				&vulkan::App::mainWindowData, vulkan::App::queueFamily, nullptr, width, height, vulkan::App::minImageCount);
			vulkan::App::rebuild();
			vulkan::App::mainWindowData.FrameIndex = 0;
			vulkan::App::swapChainRebuild = false;
		}
		if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0) {
			ImGui_ImplGlfw_Sleep(10);
			continue;
		}

		dt = io.DeltaTime;

		if (!gui::App::enabled)
			player->update();

		render();

		InputEventHandler::reset();
	}
}

void App::render() {
	gui::App::generateWindows();

	ImDrawData* main_draw_data = ImGui::GetDrawData();
	const bool mainMinimized = (main_draw_data->DisplaySize.x <= 0.0f || main_draw_data->DisplaySize.y <= 0.0f);
	if (!mainMinimized) {
		vulkan::App::beginFrame(wd);

		auto UBO = getUBO();
		vulkan::App::render(UBO);
		gui::App::render(wd);

		vulkan::App::endMainFrame(wd);
	}
	
	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}
	if (!mainMinimized) {
		vulkan::App::endFrame(wd);
	}
	if (!gui::App::enabled) {
		glfwSetCursorPos(window, 100, 100);
	}
}