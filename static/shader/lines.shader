
<VertexShader>
#version 330 core

uniform mat4 mat_mvp;

layout(location = 0) in vec3 inPosition;

void main()
{
	gl_Position = mat_mvp * vec4(inPosition,1);
}

</VertexShader>
<FragmentShader>
#version 330 core

struct Material
{
	vec4 ambient, diffusive, specular, emission;
	float shininess;
};

uniform Material material;

out vec4 color;

void main()
{
	color = material.emission;
}

</FragmentShader>

