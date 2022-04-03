<Layout>
	version = 420
</Layout>
<VertexShader>


struct Matrix {
	mat4 model, view, project;
};
/*layout(binding=0)*/ uniform Matrix matrix;

layout(location=0) in vec3 in_position;
layout(location=1) in vec3 in_normal;
layout(location=2) in vec2 in_uv;

layout(location=0) out vec4 out_pos; // view space
layout(location=1) out vec3 out_normal;
layout(location=2) out float out_weight;

void main() {
	gl_Position = matrix.project * matrix.view * matrix.model * vec4(in_position, 1);
	out_normal = (matrix.model * vec4(in_normal, 0)).xyz;
	out_pos = matrix.model * vec4(in_position, 1);
	out_weight = in_uv.x;
}



</VertexShader>
<FragmentShader>


struct Material {
	vec4 albedo, emission;
	float roughness, metal;
};
uniform Material material;

struct Light {
	mat4 proj;
	vec4 pos, dir, color;
	float radius, theta, harshness;
};
uniform int num_lights = 0;
/*layout(binding=1)*/ uniform LightData {
	Light light[32];
};

struct Matrix {
	mat4 model, view, project;
};
/*layout(binding=0)*/ uniform Matrix matrix;

layout(location=0) in vec4 in_pos; // view space
layout(location=1) in vec3 in_normal;
layout(location=2) in float in_weight;

out vec4 color;

const int COLOR_COUNT = 4;
const vec3 color_map[COLOR_COUNT] = vec3[COLOR_COUNT](
	vec3(0,0,0),
	vec3(1,0,0),
	vec3(1,1,0),
	vec3(1,1,1));
const float color_stop[COLOR_COUNT] = float[COLOR_COUNT](0, 0.4, 0.7, 1.0);

vec3 map_color(float t) {
	for (int i=1; i<COLOR_COUNT; i++)
		if (t <= color_stop[i]) {
			float a = (t - color_stop[i-1]) / (color_stop[i] - color_stop[i-1]);
			return a * color_map[i] + (1-a) * color_map[i-1];
		}
	return vec3(0,0,0);
}

void main() {
	vec3 n = normalize(in_normal);
	vec3 l = light[0].dir.xyz;
	vec3 p = in_pos.xyz / in_pos.w;
	
	vec3 eye_pos = -matrix.view[3].xyz * mat3(matrix.view);
	vec3 view_dir = normalize(p - eye_pos.xyz);
	
	vec4 c = vec4(map_color(in_weight), 1);
	//c.xyz *= 1-pow(sin(in_weight*50), 6);
	
	// emission
	color = c;
	
	// ambient
	color += c * light[0].color * material.roughness * (1 - light[0].harshness);
	
	// diffuse
	float d = max(-dot(n, l), 0);
	color += c * light[0].color * d * light[0].harshness;
	
	// specular
	vec3 H = normalize(l + view_dir);
	if (dot(n, H) < 0)
		color += light[0].color * (c + vec4(1,1,1,1))/2 * pow(-dot(n, H), 15);
	
	color.a = 0.7;// * min(in_weight * 10, 1);
}

</FragmentShader>

