<Layout>
	bindings = [[buffer,sampler]]
	pushsize = 4
	input = [vec3,vec3,vec2]
	topology = triangles
	version = 420
</Layout>
<VertexShader>
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 in_position;
layout(location = 2) in vec2 in_tex_coord;

layout(location = 0) out vec2 out_tex_coord;

void main() {
	gl_Position = vec4(in_position, 1.0);
	out_tex_coord = in_tex_coord;
}
</VertexShader>
<FragmentShader>
#extension GL_ARB_separate_shader_objects : enable

uniform vec2 axis = vec2(1,0);
uniform float radius;
uniform float kernel_r[100];
uniform float kernel_i[100];

layout(binding = 1) uniform sampler2D tex0;

layout(location = 0) in vec2 in_tex_coord;

layout(location = 0) out vec4 out_r;
layout(location = 1) out vec4 out_i;


void main() {
	//vec2 DD = 1.0 / textureSize(tex0, 0);
	ivec2 uv0 = ivec2(in_tex_coord * textureSize(tex0, 0));
	vec3 bb_r = vec3(0);
	vec3 bb_i = vec3(0);
	
	for (int i=0; i<=radius; i+=1) {
		vec3 c = texelFetch(tex0, uv0 + ivec2(axis) * i, 0).rgb;
		bb_r += kernel_r[i] * c;
		bb_i += kernel_i[i] * c;
	}
	for (int i=1; i<=radius; i+=1) {
		vec3 c = texelFetch(tex0, uv0 - ivec2(axis) * i, 0).rgb;
		bb_r += kernel_r[i] * c;
		bb_i += kernel_i[i] * c;
	}
	out_r.rgb = bb_r;
	out_i.rgb = bb_i;
}
</FragmentShader>
