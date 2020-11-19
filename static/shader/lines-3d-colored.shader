<Layout>
	version = 330 core
</Layout>
<VertexShader>

struct Matrix {
	mat4 model;
	mat4 view;
	mat4 project;
};
/*layout(binding = 0)*/ uniform Matrix matrix;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColor;

out vec4 fragmentColor;

void main() {
	gl_Position = matrix.project * matrix.view * matrix.model * vec4(inPosition, 1);
	fragmentColor = inColor;
}

</VertexShader>
<FragmentShader>

in vec4 fragmentColor;

out vec4 color;

void main() {
	color = fragmentColor;
}

</FragmentShader>
