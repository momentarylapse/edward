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
	vec4 color;
	vec4 dir;
	float radius, harshness;
};

uniform Material material;
uniform LightBlock { Light light; };

layout(location = 0) in vec3 in_normal;
out vec4 color;

void main() {
	vec3 n = normalize(in_normal);
	vec3 l = light.dir.xyz;
	float d = max(-dot(n, l), 0);
	color = material.emission;
	color += material.ambient * light.color * (1 - light.harshness);
	color += material.diffusive * light.color * d * light.harshness;
	color.a = material.diffusive.a;
}

</FragmentShader>

