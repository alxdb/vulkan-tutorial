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
        path: "resources/shaders/vert.glsl",
    }
}

mod fs {
    vulkano_shaders::shader! {
        ty: "fragment",
        path: "resources/shaders/frag.glsl",
    }
}

pub type PushConstants = vs::ty::PushConstantData;

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
