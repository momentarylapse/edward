<VertexShader>
#version 330 core

uniform mat4 mat_mvp;
uniform mat4 mat_m;
uniform mat4 mat_v;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

out vec3 fragmentNormal;

void main()
{
	gl_Position = mat_mvp * vec4(inPosition,1);
	fragmentNormal = (mat_v * mat_m * vec4(inNormal,0)).xyz;
}

</VertexShader>
<FragmentShader>
#version 330 core

struct Material
{
	vec4 ambient, diffusive, specular, emission;
	float shininess;
};

struct Light
{
	vec4 color;
	vec3 pos;
	float radius, ambient, specular;
};

uniform Material material;
uniform Light light;

in vec3 fragmentNormal;

out vec4 color;

void main()
{
	vec3 n = normalize(fragmentNormal);
	vec3 l = light.pos;
	float d = max(-dot(n, l), 0);
	color = material.emission;
	color += material.ambient * light.color * light.ambient;
	color += material.diffusive* light.color * d;
	color.a = material.diffusive.a;
}

</FragmentShader>

