use graphics::{PushConstants, Vertex};
use vulkano::buffer::{BufferUsage, CpuAccessibleBuffer};
use winit::{
    event::{Event, WindowEvent},
    event_loop::{ControlFlow, EventLoop},
};

mod graphics;

fn main() {
    let event_loop = EventLoop::new();
    let renderer = graphics::Renderer::new(&event_loop);

    let triangle_vertices = CpuAccessibleBuffer::from_iter(
        renderer.context.device.clone(),
        BufferUsage::all(),
        false,
        [
            Vertex { pos: [0.0, -0.5] },
            Vertex { pos: [0.5, 0.5] },
            Vertex { pos: [-0.5, 0.5] },
        ],
    )
    .unwrap();

    event_loop.run(move |event, _, control_flow| {
        *control_flow = ControlFlow::Wait;

        match event {
            Event::WindowEvent {
                event: WindowEvent::CloseRequested,
                ..
            } => *control_flow = ControlFlow::Exit,
            Event::RedrawEventsCleared => renderer.render(
                triangle_vertices.clone(),
                PushConstants {
                    transform: [0.5, 0.5],
                },
            ),
            _ => (),
        }
    });
}
