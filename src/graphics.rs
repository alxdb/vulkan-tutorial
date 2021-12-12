use std::sync::Arc;

use winit::{
    dpi::PhysicalSize,
    event_loop::EventLoop,
    window::{Window, WindowBuilder},
};

use vulkano_win::VkSurfaceBuild;

use vulkano::{
    device::{
        physical::{PhysicalDevice, PhysicalDeviceType},
        Device, DeviceExtensions, Features, Queue,
    },
    image::{ImageUsage, SwapchainImage},
    instance::Instance,
    render_pass::RenderPass,
    swapchain::{Surface, Swapchain},
    Version,
};

pub struct Context {
    pub instance: Arc<Instance>,
    pub surface: Arc<Surface<Window>>,
    pub device: Arc<Device>,
    pub queue: Arc<Queue>,
    pub swapchain: Arc<Swapchain<Window>>,
    pub images: Vec<Arc<SwapchainImage<Window>>>,
    pub render_pass: RenderPass,
}

impl Context {
    pub fn new(event_loop: &EventLoop<()>) -> Self {
        let instance_extensions = vulkano_win::required_extensions();
        let instance = Instance::new(None, Version::V1_0, &instance_extensions, None).unwrap();

        let surface = WindowBuilder::new()
            .with_inner_size(PhysicalSize::new(1920, 1080))
            .with_resizable(false)
            .build_vk_surface(&event_loop, instance.clone())
            .unwrap();

        let device_extensions = DeviceExtensions {
            khr_swapchain: true,
            ..DeviceExtensions::none()
        };
        let (physical_device, queue_family) = PhysicalDevice::enumerate(&instance)
            .filter(|&p| p.supported_extensions().is_superset_of(&device_extensions))
            .filter_map(|p| {
                p.queue_families().find_map(|q| {
                    (q.supports_graphics() && surface.is_supported(q).unwrap_or(false))
                        .then(|| (p, q))
                })
            })
            .min_by_key(|(p, _)| match p.properties().device_type {
                PhysicalDeviceType::DiscreteGpu => 0,
                _ => 1,
            })
            .unwrap();

        let (device, mut queues) = Device::new(
            physical_device,
            &Features::none(),
            &physical_device
                .required_extensions()
                .union(&device_extensions),
            [(queue_family, 0.5)],
        )
        .unwrap();
        let queue = queues.next().unwrap();

        let surface_capabilites = surface.capabilities(physical_device).unwrap();
        let (swapchain, images): (Arc<Swapchain<Window>>, Vec<Arc<SwapchainImage<Window>>>) =
            Swapchain::start(device.clone(), surface.clone())
                .num_images(surface_capabilites.min_image_count)
                .format(surface_capabilites.supported_formats.first().unwrap().0)
                .dimensions(surface.window().inner_size().into())
                .usage(ImageUsage::color_attachment())
                .sharing_mode(&queue)
                .composite_alpha(
                    surface_capabilites
                        .supported_composite_alpha
                        .iter()
                        .next()
                        .unwrap(),
                )
                .build()
                .unwrap();

        let render_pass = vulkano::single_pass_renderpass!(
            device.clone(),
            attachments: {
                color: {
                    load: Clear,
                    store: Store,
                    format: swapchain.format(),
                    samples: 1,
                }
            },
            pass: {
                color: [color],
                depth_stencil: {}
            }
        )
        .unwrap();

        Context {
            instance,
            surface,
            device,
            queue,
            swapchain,
            images,
            render_pass,
        }
    }
}
