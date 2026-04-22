<Layout>
	name = tessellation-evaluation-terrain
</Layout>
<Module>

//layout (quads, fractional_odd_spacing, cw) in;
layout (quads, equal_spacing, ccw) in;

#if 0
layout (binding = 0) uniform UBO {
	mat4 projection;
	mat4 model;
	float tessAlpha;
	float tessLevel;
} ubo;
#endif

layout(location = 0) in vec4 in_pos[];
layout(location = 1) in vec3 in_normal[];
layout(location = 2) in vec2 in_uv[];
layout(location = 3) in vec4 in_color[];

layout(location = 0) out vec4 out_pos;
layout(location = 1) out vec3 out_normal;
layout(location = 2) out vec2 out_uv;
layout(location = 3) out vec4 out_color;

layout(binding=4) uniform sampler2D tex4;


struct Matrices {
	mat4 model;
	mat4 view;
	mat4 project;
};

#ifdef vulkan
layout(binding = 8) uniform Parameters {
	Matrices matrix;
};
#else
/*layout(binding = 0)*/ uniform Matrices matrix;
#endif


void main() {
	float u0 = in_uv[0].x;
	float v0 = in_uv[0].y;
	float u = gl_TessCoord.x;
	float v = gl_TessCoord.y;
	float w0 = (1-u)*(1-v);
	float w1 = u*(1-v);
	float w2 = u*v;
	float w3 = (1-u)*v;
	float L = length(gl_in[1].gl_Position - gl_in[0].gl_Position);
	gl_Position = w0 * gl_in[0].gl_Position + w1 * gl_in[1].gl_Position + w2 * gl_in[2].gl_Position + w3 * gl_in[3].gl_Position;
	ivec2 size = textureSize(tex4, 0);
	int NX = size.x / 32;
	int NZ = size.y / 32;
	
	//out_normal = (matrix.view * matrix.model * vec4(0,1,0,0)).xyz;//w0*in_normal[0] + w1*in_normal[1] + w2*in_normal[2] + w3*in_normal[3];
	out_uv = (w0*in_uv[0] + w1*in_uv[1] + w2*in_uv[2] + w3*in_uv[3]) / 32;
	out_pos = w0*in_pos[0] + w1*in_pos[1] + w2*in_pos[2] + w3*in_pos[3];
	
	float h = texture(tex4, vec2((u0+u) / NX, (v0+v) / NZ).yx).r;
	float hx = texture(tex4, vec2((u0+u+0.2) / NX, (v0+v) / NZ).yx).r;
	float hy = texture(tex4, vec2((u0+u) / NX, (v0+v+0.2) / NZ).yx).r;
	out_normal = (matrix.view * matrix.model * vec4(normalize(vec3(h-hx, L*0.2, h-hy)),0)).xyz;
	//out_normal = (matrix.view * matrix.model * vec4(normalize(vec3(0, L, 0)),0)).xyz;
//	gl_Position.xyz += out_normal * 10000 * u*(1-u) * v*(1-v);
//	gl_Position.xyz += out_normal * h*4;
	gl_Position += matrix.view * matrix.model * vec4(0,h,0,0);
	out_pos = gl_Position;
	gl_Position = matrix.project * gl_Position;
}

</Module>
