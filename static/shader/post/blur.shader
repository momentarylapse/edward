<Layout>
	bindings = [[sampler]]
	pushsize = 96
	input = [vec3,vec3,vec2]
	topology = triangles
	version = 420
</Layout>
<VertexShader>

layout(location = 0) in vec3 in_position;
layout(location = 2) in vec2 in_tex_coord;
layout(location = 0) out vec2 out_tex_coord;

void main() {
	gl_Position = vec4(in_position, 1.0);
	out_tex_coord = in_tex_coord;
}
</VertexShader>
<FragmentShader>

#ifdef vulkan
//layout(binding=0) uniform Parameters {
layout(push_constant) uniform PushConstants {
	vec2 axis;
	float radius;
	float threshold;
	float kernel[20];
};
#else
uniform vec2 axis = vec2(1,1);
uniform float radius = 10.0;
uniform float threshold = 0.0;
uniform float kernel[20];
#endif

layout(binding = 0) uniform sampler2D tex0;
layout(location = 0) in vec2 in_tex_coord;
layout(location = 0) out vec4 out_color;


float brightness(vec3 c) {
	return dot(c, vec3(0.2126, 0.7152, 0.0722));
}

vec3 blur() {
	//vec2 DD = 1.0 / textureSize(tex0, 0);
	ivec2 uv0 = ivec2(in_tex_coord * textureSize(tex0, 0));
	vec3 bb = vec3(0,0,0);
	float sum = 0.0;
	
	int R = int(radius);
	int RR = max(R-1, 0);
	for (int i=-RR; i<=RR; i+=1) {
		float fi = i;
		//float w = kernel[i + RR];
		float w = exp(-((fi*fi) / (radius*radius)) * 3.3);
		vec3 c = texelFetch(tex0, uv0 + ivec2(axis * fi), 0).rgb;
		float br = brightness(c);
		if (br > threshold) {
			bb += w * c;// * (br - 1);
		}
		sum += w;
	}
	return bb / sum;
}

void main() {
	out_color.rgb = blur();
	out_color.a = 1;
}
</FragmentShader>
