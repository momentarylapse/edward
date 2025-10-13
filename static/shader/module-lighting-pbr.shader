<Layout>
	version = 420
	name = lighting
</Layout>
<Module>

// import basic-data first!

#import light-sources
#import shadows

// https://learnopengl.com/PBR/Theory
// https://learnopengl.com/PBR/Lighting

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

vec3 _surf_specular(vec3 albedo, float metal, float roughness, vec3 V, vec3 L, vec3 n, out vec3 F) {

	vec3 F0 = vec3(0.04);
	F0 = mix(F0, albedo, metal);
        
        vec3 H = normalize(V + L);
        
        // cook-torrance brdf
        float NDF = DistributionGGX(n, H, roughness);
        float G   = GeometrySmith(n, V, L, roughness);
        F         = fresnelSchlick(max(dot(H, V), 0.0), F0);
        
        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metal;
        
        vec3 numerator    = NDF * G * F;
        float denominator = 4.0 * max(dot(n, V), 0.0) * max(dot(n, L), 0.0);
        return numerator / max(denominator, 0.001);
}

vec3 _surf_light_add(Light l, vec3 p, vec3 n, vec3 albedo, float metal, float roughness, float ambient_occlusion, vec3 view_dir, bool with_shadow) {
	float shadow_factor = 1.0;
	if (with_shadow)
		shadow_factor = _shadow_factor(l, p);
		
	// TODO only affect "diffuse"
	shadow_factor *= (1-ambient_occlusion);


	// calculate per-light radiance
	vec3 radiance = l.color.rgb * _light_source_brightness(l, p) * PI * shadow_factor;
	
	if (l.theta > 3.0) {
		return albedo / PI * radiance;
	}
        
        vec3 V = -view_dir;
        vec3 L = -_light_source_dir(l, p);
        
        vec3 F;
        vec3 specular = _surf_specular(albedo, metal, roughness, V, L, n, F);
        
        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metal;
            
        // add to outgoing radiance Lo
        float NdotL = max(dot(n, L), 0.0);
        return (kD * albedo / PI + specular) * radiance * NdotL;
}

vec4 perform_lighting(vec3 p, vec3 n, vec4 albedo, vec4 emission, float metal, float roughness0, float ambient_occlusion, vec3 eye_pos) {
	vec3 view_dir = normalize(p - eye_pos);
	
	float roughness = max(roughness0, 0.03);
	
	
	vec4 color = emission;
	
	
#if 0
		/*mat3 R = transpose(mat3(matrix.view));
		vec3 L = reflect(view_dir, n);
		//for (int i=0; i<30; i++) {
		//vec3 L = normalize(L0 + vec3(_surf_rand3d(p)-0.5, _surf_rand3d(p)-0.5, _surf_rand3d(p)-0.5) / 50);
	//out_color = texture(tex_cube, reflect(view_dir, n));
	out_color = texture(tex_cube, R * n);
	return;*/
	

	//if (metal > 0.01 && false) {
	if (metal > 0.9 && roughness < 0.2) {
		mat3 R = transpose(mat3(matrix.view));
		vec3 L = reflect(view_dir, n);
		//for (int i=0; i<30; i++) {
		//vec3 L = normalize(L0 + vec3(_surf_rand3d(p)-0.5, _surf_rand3d(p)-0.5, _surf_rand3d(p)-0.5) / 50);
		vec4 r = texture(tex_cube, R*L);
		//out_color = r;
		//return;
		/*if (roughness > 0.1) {
			r += texture(tex_cube, reflect(view_dir, normalize(n + vec3(_surf_rand3d(p),0,1) * roughness/10)));
			r += texture(tex_cube, reflect(view_dir, normalize(n + vec3(1,_surf_rand3d(p),0) * roughness/10)));
			r += texture(tex_cube, reflect(view_dir, normalize(n + vec3(0,1,_surf_rand3d(p)) * roughness/10)));
			r /= 5;
		}
		out_color += r * reflectivity;*/
		
		
		
	//	float R = (1-roughness) + roughness * pow(1 - dot(n, L), 5);
	//	out_color.rgb += R * r.rgb;

		vec3 F;
        	vec3 specular = min(_surf_specular(albedo.rgb, metal, roughness, -view_dir, L, n, F), vec3(1));
        	float NdotL = max(dot(n, L), 0.0);
        	color.rgb += specular * r.rgb * NdotL;
        	//out_color.rgb = specular * NdotL;
        	return color;
	}
#endif
	

	for (int i=0; i<num_lights; i++)
		if (_light_source_reachable(light[i], p))
			color.rgb += _surf_light_add(light[i], p, n, albedo.rgb, metal, roughness, ambient_occlusion, view_dir, light[i].shadow_index >= 0).rgb;
	
/*	float distance = length(p - eye_pos.xyz);
	float f = exp(-distance / fog.distance);
	out_color.rgb = f * out_color.rgb + (1-f) * fog.color.rgb;
	
	*/
	
	
	if (roughness0 < 0.2 && metal > 0.8) {
		if (textureSize(tex_cube, 0).x > 10) {
			vec3 p = in_pos.xyz / in_pos.w;
			mat3 R = transpose(mat3(matrix.view));
			vec3 L = reflect(p, n);
			vec4 cube = texture(tex_cube, R*L);
			color += cube * ((metal-0.8) / 0.2) * ((0.2 - roughness0) / 0.2) * 0.5;
		}
	}
	
	
	color.a = albedo.a;
	return color;
}
</Module>
