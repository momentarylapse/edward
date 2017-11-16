<VertexShader>
#version 330 core

uniform mat4 mat_mvp;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColor;

out vec4 fragmentColor;

void main()
{
	gl_Position = mat_mvp * vec4(inPosition,1);
	fragmentColor = inColor;
}

</VertexShader>
<FragmentShader>
#version 330 core

in vec4 fragmentColor;

out vec4 color;

void main()
{
	color = fragmentColor;
}

</FragmentShader>
