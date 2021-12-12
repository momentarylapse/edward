<Layout>
	version = 420
</Layout>
<VertexShader>

struct Matrix {
	mat4 model;
	mat4 view;
	mat4 project;
};
/*layout(binding = 0)*/ uniform Matrix matrix;

layout(location=0) in vec3 in_pos;
layout(location=1) in vec4 in_color;

layout(location=0) out vec4 out_color;

void main() {
	gl_Position = matrix.project * matrix.view * matrix.model * vec4(in_pos, 1);
	out_color = in_color;
}

</VertexShader>
<GeometryShader>

layout (lines) in;
layout (triangle_strip, max_vertices = 4) out;

layout(location=0) in vec4 in_color[];
layout(location=0) out vec4 out_color;

uniform float target_width = 1024, target_height = 768;
uniform float line_width = 4;

void main() {
	float w0 = gl_in[0].gl_Position.w;
	float w1 = gl_in[1].gl_Position.w;
	vec2 d = (gl_in[1].gl_Position.xy / w1 - gl_in[0].gl_Position.xy / w0);
	d = vec2(d.x * target_width/2, d.y * target_height/2);
	d = vec2(d.y, -d.x) / length(d) * line_width/2;
	d = vec2(d.x / target_width*2, d.y / target_height*2);

	gl_Position = gl_in[0].gl_Position - vec4(d*w0,0,0);
	out_color = in_color[0];
	EmitVertex();

	gl_Position = gl_in[1].gl_Position - vec4(d*w1,0,0);
	out_color = in_color[1];
	EmitVertex();
    
	gl_Position = gl_in[0].gl_Position + vec4(d*w0,0,0);
	out_color = in_color[0];
	EmitVertex();

	gl_Position = gl_in[1].gl_Position + vec4(d*w1,0,0);
	out_color = in_color[1];
	EmitVertex();
    
	EndPrimitive();
}

</GeometryShader>
<FragmentShader>

layout(location=0) in vec4 in_color;

out vec4 out_color;

void main() {
	out_color = in_color;
}

</FragmentShader>

