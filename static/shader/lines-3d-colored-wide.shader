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

out vec4 fragmentColor0;

void main() {
	gl_Position = matrix.project * matrix.view * matrix.model * vec4(inPosition, 1);
	fragmentColor0 = inColor;
}

</VertexShader>
<GeometryShader>

layout (lines) in;
layout (triangle_strip, max_vertices = 4) out;

in vec4 fragmentColor0[];
out vec4 fragmentColor;

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
	fragmentColor = fragmentColor0[0];
	EmitVertex();

	gl_Position = gl_in[1].gl_Position - vec4(d*w1,0,0);
	fragmentColor = fragmentColor0[1];
	EmitVertex();
    
	gl_Position = gl_in[0].gl_Position + vec4(d*w0,0,0);
	fragmentColor = fragmentColor0[0];
	EmitVertex();

	gl_Position = gl_in[1].gl_Position + vec4(d*w1,0,0);
	fragmentColor = fragmentColor0[1];
	EmitVertex();
    
	EndPrimitive();
}

</GeometryShader>
<FragmentShader>

in vec4 fragmentColor;

out vec4 color;

void main() {
	color = fragmentColor;
}

</FragmentShader>

