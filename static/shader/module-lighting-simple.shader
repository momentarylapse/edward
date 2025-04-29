<Layout>
	version = 420
	name = lighting
</Layout>
<Module>

#import light-sources

vec3 _surf_light_add(Light l, vec3 p, vec3 n, vec3 albedo, float metal, float roughness, float ambient_occlusion, vec3 view_dir) {
	// calculate per-light radiance
	vec3 radiance = l.color.rgb * _light_source_brightness(l, p);

	vec3 V = -view_dir;
	vec3 L = -_light_source_dir(l, p);

	// add to outgoing radiance Lo
	float NdotL = max(dot(n, L), 0.0);
	return albedo * radiance * NdotL;
}

vec4 perform_lighting(vec3 p, vec3 n, vec4 albedo, vec4 emission, float metal, float roughness, float ambient_occlusion, vec3 eye_pos) {
	vec3 view_dir = normalize(p - eye_pos);
	
	roughness = max(roughness, 0.03);

	vec4 color = emission;
	for (int i=0; i<num_lights; i++)
		if (_light_source_reachable(light[i], p))
			color.rgb += _surf_light_add(light[i], p, n, albedo.rgb, metal, roughness, ambient_occlusion, view_dir).rgb;
	
/*	float distance = length(p - eye_pos.xyz);
	float f = exp(-distance / fog.distance);
	out_color.rgb = f * out_color.rgb + (1-f) * fog.color.rgb;
	
	*/
	color.a = albedo.a;
	return color;
}
</Module>
