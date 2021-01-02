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

void main() {
	gl_Position = matrix.project * matrix.view * matrix.model * vec4(inPosition, 1);
}

</VertexShader>
<FragmentShader>

struct Material {
	vec4 albedo, emission;
	float roughness, metal;
};
/*layout(binding = 2)*/ uniform Material material;

out vec4 color;

void main() {
	color = material.emission;
}

</FragmentShader>

