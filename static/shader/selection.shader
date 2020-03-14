<VertexShader>
#version 330 core
#extension GL_ARB_separate_shader_objects : enable

uniform mat4 mat_mvp;
uniform mat4 mat_m;
uniform mat4 mat_v;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;

layout(location = 0) out vec3 out_normal;

void main() {
	gl_Position = mat_mvp * vec4(in_position,1);
	out_normal = (mat_v * mat_m * vec4(in_normal,0)).xyz;
}

</VertexShader>
<FragmentShader>
#version 330 core
#extension GL_ARB_separate_shader_objects : enable

struct Material {
	vec4 ambient, diffusive, specular, emission;
	float shininess;
};

struct Light {
	mat4 proj;
	vec4 pos, dir, color;
	float radius, theta, harshness;
};
uniform int num_lights = 0;
/*layout(binding = 1)*/ uniform LightData { Light light[32]; };

uniform Material material;

layout(location = 0) in vec3 in_normal;
out vec4 color;

void main() {
	vec3 n = normalize(in_normal);
	vec3 l = light[0].dir.xyz;
	float d = max(-dot(n, l), 0);
	color = material.emission;
	color += material.ambient * light[0].color * (1 - light[0].harshness);
	color += material.diffusive * light[0].color * d * light[0].harshness;
	color.a = material.diffusive.a;
}

</FragmentShader>

