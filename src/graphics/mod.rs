mod context;
mod pipeline;

use std::sync::Arc;

use context::Context;
use pipeline::Pipeline;
use vulkano::{
    buffer::{CpuAccessibleBuffer, TypedBufferAccess},
    command_buffer::{AutoCommandBufferBuilder, CommandBufferUsage, SubpassContents},
    pipeline::Pipeline as VulkanoPipeline,
    swapchain,
    sync::GpuFuture,
};
use winit::event_loop::EventLoop;

pub use pipeline::{PushConstants, Vertex};

pub struct Renderer {
    pub context: Context,
    pipeline: Pipeline,
}

impl Renderer {
    pub fn new(event_loop: &EventLoop<()>) -> Self {
        let context = Context::new(event_loop);
        let pipeline = Pipeline::new(&context);
        Self { context, pipeline }
    }

    pub fn render(
        &self,
        vertex_buffer: Arc<CpuAccessibleBuffer<[Vertex]>>,
        push_constants: PushConstants,
    ) {
        let (image_num, _, acquire_future) =
            swapchain::acquire_next_image(self.context.swapchain.clone(), None).unwrap();

        let mut command_buffer_builder = AutoCommandBufferBuilder::primary(
            self.context.device.clone(),
            self.context.queue.family(),
            CommandBufferUsage::OneTimeSubmit,
        )
        .unwrap();
        command_buffer_builder
            .push_constants(self.pipeline.pipeline.layout().clone(), 0, push_constants)
            .begin_render_pass(
                self.context.framebuffers[image_num].clone(),
                SubpassContents::Inline,
                vec![[0.0, 0.0, 0.0, 1.0].into()],
            )
            .unwrap()
            .set_viewport(0, [self.context.viewport.clone()])
            .bind_pipeline_graphics(self.pipeline.pipeline.clone())
            .bind_vertex_buffers(0, vertex_buffer.clone())
            .draw(vertex_buffer.len() as u32, 1, 0, 0)
            .unwrap()
            .end_render_pass()
            .unwrap();
        let command_buffer = command_buffer_builder.build().unwrap();

        acquire_future
            .then_execute(self.context.queue.clone(), command_buffer)
            .unwrap()
            .then_swapchain_present(
                self.context.queue.clone(),
                self.context.swapchain.clone(),
                image_num,
            )
            .then_signal_fence_and_flush()
            .unwrap();
    }
}
