#include "window.hh"

class VulkanApp {
	Window window;
	// Instance
	vk::UniqueInstance instance;
	// Surface
	vk::UniqueSurfaceKHR surface;
	// Physical Device
	vk::PhysicalDevice physical_device;
	uint32_t graphics_queue_family;
	// Logical Device
	vk::UniqueDevice device;
	vk::Queue graphics_queue;
	// Swapchain
	vk::UniqueSwapchainKHR swapchain;
	// Image Views
	std::vector<vk::UniqueImageView> swapchain_image_views;
	// Render Pass
	vk::UniqueRenderPass render_pass;
	// Graphics Pipeline
	vk::UniqueShaderModule vert_shader;
	vk::UniqueShaderModule frag_shader;
	vk::UniquePipelineLayout pipeline_layout;
	vk::UniquePipeline graphics_pipeline;
	// Framebuffers
	std::vector<vk::UniqueFramebuffer> swapchain_frame_buffers;
	// Command Pool
	vk::UniqueCommandPool command_pool;
	// Command Buffers
	std::vector<vk::UniqueCommandBuffer> command_buffers;
	// Sync Objects
	std::vector<vk::UniqueSemaphore> image_available_semaphores;
	std::vector<vk::UniqueSemaphore> render_finished_semaphores;
	std::vector<vk::UniqueFence> in_flight_fences;
	std::vector<vk::Fence> images_in_flight;
	size_t current_frame = 0;

	void render();
	void create_swapchain();
	bool poll_events();
public:
	void run();
	explicit VulkanApp(const char* application_name, bool debug = true);
	~VulkanApp();
};
