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
<FragmentShader>

layout(location=0) in vec4 in_color;

out vec4 out_color;

void main() {
	out_color = in_color;
}

</FragmentShader>
