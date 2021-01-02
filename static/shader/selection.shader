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

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;

layout(location = 0) out vec3 out_normal;

void main() {
	gl_Position = matrix.project * matrix.view * matrix.model * vec4(in_position, 1);
	out_normal = (matrix.view * matrix.model * vec4(in_normal, 0)).xyz;
}

</VertexShader>
<FragmentShader>


struct Material {
	vec4 albedo, emission;
	float roughness, metal;
};
/*layout(binding = 2)*/ uniform Material material;

struct Light {
	mat4 proj;
	vec4 pos, dir, color;
	float radius, theta, harshness;
};
uniform int num_lights = 0;
/*layout(binding = 1)*/ uniform LightData { Light light[32]; };

layout(location = 0) in vec3 in_normal;
out vec4 color;

void main() {
	vec3 n = normalize(in_normal);
	vec3 l = light[0].dir.xyz;
	float d = max(-dot(n, l), 0);
	color = material.emission;
	color += material.albedo * material.roughness * light[0].color * (1 - light[0].harshness);
	color += material.albedo * light[0].color * d * light[0].harshness;
	color.a = material.albedo.a;
}

</FragmentShader>

