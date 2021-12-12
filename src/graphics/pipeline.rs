use std::sync::Arc;

use vulkano::{
    pipeline::{
        graphics::{
            input_assembly::InputAssemblyState, vertex_input::BuffersDefinition,
            viewport::ViewportState,
        },
        GraphicsPipeline,
    },
    render_pass::Subpass,
};

use super::Context;

#[repr(C)]
#[derive(Default, Debug, Clone)]
pub struct Vertex {
    pub pos: [f32; 2],
}
vulkano::impl_vertex!(Vertex, pos);

mod vs {
    vulkano_shaders::shader! {
        ty: "vertex",
        src: "
                #version 450
                layout(location = 0) in vec2 pos;
                void main() {
                    gl_Position = vec4(pos, 0.0, 1.0);
                }
            "
    }
}

mod fs {
    vulkano_shaders::shader! {
        ty: "fragment",
        src: "
                #version 450
                layout(location = 0) out vec4 f_color;
                void main() {
                    f_color = vec4(1.0, 1.0, 1.0, 1.0);
                }
            "
    }
}

pub struct Pipeline {
    pub pipeline: Arc<GraphicsPipeline>,
}

impl Pipeline {
    pub fn new(context: &Context) -> Self {
        let vs = vs::load(context.device.clone()).unwrap();
        let fs = fs::load(context.device.clone()).unwrap();

        let pipeline = GraphicsPipeline::start()
            .vertex_input_state(BuffersDefinition::new().vertex::<Vertex>())
            .vertex_shader(vs.entry_point("main").unwrap(), ())
            .input_assembly_state(InputAssemblyState::new())
            .viewport_state(ViewportState::viewport_dynamic_scissor_irrelevant())
            .fragment_shader(fs.entry_point("main").unwrap(), ())
            .render_pass(Subpass::from(context.render_pass.clone(), 0).unwrap())
            .build(context.device.clone())
            .unwrap();

        Pipeline { pipeline }
    }
}
