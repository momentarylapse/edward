<Layout>
	version = 420
	bindings = [[sampler]]
</Layout>
<VertexShader>


layout(location=0) in vec3 in_position;
layout(location=1) in vec3 in_normal;
layout(location=2) in vec2 in_uv;

layout(location=2) out vec2 out_uv;

void main() {
	gl_Position = vec4(in_position, 1);
	out_uv = in_uv;
}

</VertexShader>
<FragmentShader>

layout(location=2) in vec2 in_uv;

layout(binding=0) uniform sampler2D tex0;

layout(location=0) out vec4 out_color;

void main() {
	out_color = texture(tex0, in_uv);
	out_color.a = 1;
	
	// gamma correction (gtk does not auto-correct linear -> sRGB)
	out_color.r = pow(out_color.r, 1.0/2.2);
	out_color.g = pow(out_color.g, 1.0/2.2);
	out_color.b = pow(out_color.b, 1.0/2.2);
}
</FragmentShader>
