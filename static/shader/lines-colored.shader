<VertexShader>
#version 330 core
uniform mat4 mat_mvp;
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColor;
out vec4 fragmentColor0;
void main(){
	gl_Position = mat_mvp * vec4(inPosition,1);
	fragmentColor0 = inColor;
}
</VertexShader>
<GeometryShader>
#version 330 core
layout (lines) in;
layout (triangle_strip, max_vertices = 4) out;

in vec4 fragmentColor0[];
out vec4 fragmentColor;

void main() {

	float w0 = gl_in[0].gl_Position.w;
	float w1 = gl_in[1].gl_Position.w;
	vec2 d = (gl_in[1].gl_Position.xy / w1 - gl_in[0].gl_Position.xy / w0);
	d = vec2(d.y, -d.x) / length(d) * 0.002;

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
#version 330 core
in vec4 fragmentColor;
out vec4 color;
void main(){
	color = fragmentColor;
}
</FragmentShader>
