<Layout>
	bindings = [[sampler,sampler]]
	pushsize = 8
	input = [vec3,vec3,vec2]
	topology = triangles
	version = 420
</Layout>
<VertexShader>
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 in_position;
layout(location = 2) in vec2 in_tex_coord;

//layout(location = 0) out vec4 out_pos;
layout(location = 0) out vec2 out_tex_coord;

void main() {
	gl_Position = vec4(in_position, 1.0);
	out_tex_coord = in_tex_coord;
}
</VertexShader>
<FragmentShader>
#extension GL_ARB_separate_shader_objects : enable

#ifdef vulkan
layout(push_constant) uniform PushConstants {
	float width;
	float height;
};
#else
uniform float width = 1920.0;
uniform float height = 1080.0;
#endif

layout(binding = 0) uniform sampler2DMS tex0;

layout(location = 0) in vec2 in_tex_coord;

layout(location = 0) out vec4 out_color;

void main() {
	ivec2 p = ivec2(in_tex_coord.x*width, in_tex_coord.y*height);
	out_color  = texelFetch(tex0, p, 0);
	out_color += texelFetch(tex0, p, 1);
	out_color += texelFetch(tex0, p, 2);
	out_color += texelFetch(tex0, p, 3);
	out_color /= 4;
}
</FragmentShader>
