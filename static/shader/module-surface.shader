<Layout>
	version = 420
	name = surface
</Layout>
<Module>


uniform vec3 eye_pos;


struct Light {
	mat4 proj;
	vec4 pos;
	vec4 dir;
	vec4 color;
	float radius, theta, harshness;
};
uniform int num_lights;
//uniform int shadow_index = 0;

/*layout(binding = 1)*/ uniform LightData {
	Light light[32];
};

struct Fog {
	vec4 color;
	float distance;
};
/*layout(binding = 3)*/ uniform Fog fog;



layout(location = 0) in vec4 in_pos; // world
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec3 in_normal;

layout(location = 0) out vec4 out_color;


vec3 _surf_light_dir(Light l, vec3 p) {
	if (l.radius < 0)
		return l.dir.xyz;
	return normalize(p - l.pos.xyz);
}


float _surf_brightness(Light l, vec3 p, vec3 n) {
	// parallel
	if (l.radius < 0)
		return 1.0f;
	
	
	float d = length(p - l.pos.xyz);
	if (d > l.radius)
		return 0.0;
	float b = min(pow(1.0/d, 2), 1.0);
	
	// spherical
	if (l.theta < 0)
		return b;
	
	float t = acos(dot(l.dir.xyz, normalize(p - l.pos.xyz)));
	float tmax = l.theta;
	return b * (1 - smoothstep(tmax*0.8, tmax, t));
}

// blinn phong
float _surf_specular(Light l, vec3 p, vec3 n, float shininess) {
	vec3 view_dir = normalize(p - eye_pos.xyz);
	vec3 half_dir = normalize(_surf_light_dir(l, p) + view_dir);

	float spec_angle = max(-dot(half_dir, n), 0.0);
	return pow(spec_angle, shininess);
}


vec3 _surf_light_add(Light l, vec3 p, vec3 n, vec3 t, float sp, float shininess) {
	float shadow = 1.0;
	
	

	float b = _surf_brightness(l, p, n) * shadow;
	float lambert = max(-dot(n, _surf_light_dir(l, p)), 0);
	
	float bb = l.harshness * lambert + (1 - l.harshness);
	vec3 col = t * l.color.rgb * bb * b;
	
	// specular
	if (lambert > 0 && sp*shininess > 0) {
		float spx = sp * _surf_specular(l, p, n, shininess);
		col += spx * l.color.rgb * b;
	}
	return col;
}

void surface_out(vec3 n, vec4 di, vec4 em, float sp, float shininess) {
	out_color = em;
	vec3 p = in_pos.xyz / in_pos.w;
	
///	float reflectivity = 1-((1-xxx.x) * (1-exp(-pow(dot(d, n),2) * 100)));
	float reflectivity = 0.0;
	

	for (int i=0; i<num_lights; i++)
		out_color.rgb += _surf_light_add(light[i], p, n, di.rgb, sp, shininess).rgb;
	
/*	float distance = length(p - eye_pos.xyz);
	float f = exp(-distance / fog.distance);
	out_color.rgb = f * out_color.rgb + (1-f) * fog.color.rgb;
	
	*/
	
	out_color.a = di.a;
}
</Module>
