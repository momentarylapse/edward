<Layout>
	version = 420
</Layout>
<VertexShader>
#import vertex-default
</VertexShader>
<FragmentShader>

#import basic-data

void main() {
	vec3 n = normalize(in_normal);
	vec3 l = light[0].dir.xyz;
	vec3 p = in_pos.xyz / in_pos.w;
	
	vec3 eye_pos = -matrix.view[3].xyz * mat3(matrix.view);
	vec3 view_dir = normalize(p - eye_pos.xyz);
	
	// emission
	out_color = material.emission;
	
	// ambient
	out_color += material.albedo * light[0].color * material.roughness * (1 - light[0].harshness);
	
	// diffuse
	float d = max(-dot(n, l), 0);
	out_color += material.albedo * light[0].color * d * light[0].harshness;
	
	// specular
	vec3 H = normalize(l + view_dir);
	if (dot(n, H) < 0)
		out_color += light[0].color * (material.emission + vec4(1,1,1,1))/2 * pow(-dot(n, H), 20);
	
	out_color.a = material.albedo.a;
}

</FragmentShader>

