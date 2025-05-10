<Layout>
	version = 420
	name = basic-interface
</Layout>
<Module>

struct Matrices {
	mat4 model;
	mat4 view;
	mat4 project;
};

struct Material {
	vec4 albedo, emission;
	float roughness, metal;
	int _dummy1, _dummy2;
};

struct Light {
	vec4 pos;
	vec4 dir;
	vec4 color;
	float radius, theta, harshness;
	int shadow_index;
};

struct Surfel {
	vec4 pos; // w=r
	vec4 n;   // w=rz
	vec4 color;
};


layout(binding=0) uniform sampler2D tex0;
layout(binding=1) uniform sampler2D tex1;
layout(binding=2) uniform sampler2D tex2;
layout(binding=3) uniform sampler2D tex3;
layout(binding=5) uniform sampler2D tex_shadow0;
layout(binding=6) uniform sampler2D tex_shadow1;
layout(binding=7) uniform samplerCube tex_cube;


#ifdef vulkan

layout(binding=8) uniform ParameterData {
	Matrices matrix;
	Material material;
};

#else

uniform Material material;
uniform Matrices matrix;

#endif

layout(binding=9, std140) uniform LightData {
	int num_lights;
	int num_surfels;
	int _dummy0, _dummy1;
	mat4 shadow_proj[2];
	Light light[500];
};
layout(binding=10) uniform Multi {
	mat4 multi[1024];
};
layout(binding=11) uniform BoneData {
	mat4 bone_matrix[1024];
};
layout(binding=12) uniform SurfelData {
	Surfel surfels[1024];
};

//uniform Fog fog;



//uniform vec3 eye_pos;
const vec3 eye_pos = vec3(0,0,0);


const float PI = 3.141592654;




float _surf_rand3d(vec3 p) {
	return fract(sin(dot(p ,vec3(12.9898,78.233,4213.1234))) * 43758.5453);
}

vec4 texture_blurred(sampler2D tex, vec2 uv, float r) {
	const int N = 12;
	vec4 c = texture(tex, uv);
	for (int i=1; i<N; i++) {
		float fi = i;
		c += texture(tex, uv + vec2(sin(i), cos(i)) * r * fi / N);
	}
	return c / N;
}



</Module>
