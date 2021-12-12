<Layout>
	version = 420
</Layout>
<VertexShader>
#import vertex-default
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
/*layout(binding = 1)*/ uniform LightData { Light light[32]; };

struct Matrix {
	mat4 model, view, project;
};
/*layout(binding = 0)*/ uniform Matrix matrix;

layout(location=0) in vec4 in_pos; // view space
layout(location=1) in vec3 in_normal;
layout(location=2) in vec2 in_uv;

out vec4 color;

void main() {
	vec3 n = normalize(in_normal);
	vec3 l = light[0].dir.xyz;
	vec3 p = in_pos.xyz / in_pos.w;
	
	vec3 eye_pos = -matrix.view[3].xyz * mat3(matrix.view);
	vec3 view_dir = normalize(p - eye_pos.xyz);
	
	// emission
	color = material.emission;
	
	// ambient
	color += material.albedo * light[0].color * material.roughness * (1 - light[0].harshness);
	
	// diffuse
	float d = max(-dot(n, l), 0);
	color += material.albedo * light[0].color * d * light[0].harshness;
	
	// specular
	vec3 H = normalize(l + view_dir);
	if (dot(n, H) < 0)
		color += light[0].color * (material.emission + vec4(1,1,1,1))/2 * pow(-dot(n, H), 20);
	
	color.a = material.albedo.a;
}

</FragmentShader>

