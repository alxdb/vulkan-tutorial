#include "window.hh"
#include "vertex.hh"

class VulkanApp {
	Window window;
	vk::UniqueInstance instance;
	vk::UniqueSurfaceKHR surface;
	// Device
	vk::PhysicalDevice physical_device;
	uint32_t graphics_queue_family;
	vk::UniqueDevice device;
	vk::Queue graphics_queue;
	// Swapchain
	vk::UniqueSwapchainKHR swapchain;
	std::vector<vk::UniqueImageView> swapchain_image_views;
	std::vector<vk::UniqueFramebuffer> swapchain_frame_buffers;
	// Graphics Pipeline
	vk::UniqueRenderPass render_pass;
	vk::UniqueShaderModule vert_shader;
	vk::UniqueShaderModule frag_shader;
	vk::UniquePipelineLayout pipeline_layout;
	vk::UniquePipeline graphics_pipeline;
	// Vertex Buffer
	std::vector<Vertex> vertices;
	vk::UniqueBuffer vertex_buffer;
	vk::UniqueDeviceMemory vertex_memory;
	// Commands
	vk::UniqueCommandPool command_pool;
	std::vector<vk::UniqueCommandBuffer> command_buffers;
	// Sync
	std::vector<vk::UniqueSemaphore> image_available_semaphores;
	std::vector<vk::UniqueSemaphore> render_finished_semaphores;
	std::vector<vk::UniqueFence> in_flight_fences;
	std::vector<vk::Fence> images_in_flight;
	size_t max_frames_in_flight = 2;
	size_t current_frame = 0;

	void render();
	void generate_swapchain();
	bool poll_events();
public:
	void run();
	explicit VulkanApp(
		const char* application_name,
		std::vector<Vertex> init_vertices = {},
		bool debug = true
	);
	void set_vertices(const std::vector<Vertex>& vertices) { this->vertices = vertices; };
	~VulkanApp();
};
