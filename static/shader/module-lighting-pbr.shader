<Layout>
	version = 420
	name = lighting
</Layout>
<Module>

// import basic-data first!

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



float _surf_rand3d(vec3 p) {
	return fract(sin(dot(p ,vec3(12.9898,78.233,4213.1234))) * 43758.5453);
}

vec3 _surf_light_dir(Light l, vec3 p) {
	if (l.radius < 0)
		return l.dir.xyz;
	return normalize(p - l.pos.xyz);
}


float _surf_brightness(Light l, vec3 p) {
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
	
	// cone
	float t = acos(dot(l.dir.xyz, normalize(p - l.pos.xyz)));
	float tmax = l.theta;
	return b * (1 - smoothstep(tmax*0.8, tmax, t));
}

// amount of shadow
float _surf_shadow_pcf_step(vec3 p, vec2 dd, ivec2 ts) {
	vec2 d = dd / ts * 0.8;
	vec2 tp = p.xy + d;
	float epsilon = 0.004;
	float shadow_z = texture(tex_shadow1, p.xy + d).r + epsilon;
	if (tp.x > 0.38 && tp.y > 0.38 && tp.x < 0.62 && tp.y < 0.62)
		shadow_z = texture(tex_shadow0, (p.xy - vec2(0.5,0.5))*4 + vec2(0.5,0.5) + d).r + epsilon;
	if (p.z > shadow_z)
		return 1.0;
	return 0.0;
}

vec2 VogelDiskSample(int sampleIndex, int samplesCount, float phi) {
	float GoldenAngle = 2.4;

	float r = sqrt(sampleIndex + 0.5) / sqrt(samplesCount);
	float theta = sampleIndex * GoldenAngle + phi;
	return vec2(r * cos(theta), r * sin(theta));
}

float _surf_shadow_pcf(vec3 p) {
	ivec2 ts = textureSize(tex_shadow0, 0);
	float value = 0;//shadow_pcf_step(p, vec2(0,0), ts);
	const float R = 1.8;
	const int N = 16;
	float phi0 = _surf_rand3d(p) * 2 * 3.1415;
	for (int i=0; i<N; i++) {
		//float phi = _surf_rand3d(p + p*i) * 2 * 3.1415;
		//float r = R * sqrt(fract(phi * 235.3545));
		//vec2 dd = r * vec2(cos(phi), sin(phi));
		vec2 dd = VogelDiskSample(i, N, phi0) * R;
		value += _surf_shadow_pcf_step(p, dd, ts);
	}
	return value / N;
}

vec3 _surf_light_proj(Light l, vec3 p) {
	vec4 proj = l.proj * vec4(p,1);
	proj.xyz /= proj.w;
	proj.x = (proj.x +1)/2;
	proj.y = (proj.y +1)/2;
#ifdef vulkan
	proj.y = 1 - proj.y;
#endif
	//proj.z = (proj.z +1)/2;
	return proj.xyz;
}

float _surf_shadow_factor(Light l, vec3 p) {
	vec3 proj = _surf_light_proj(l, p);
	
	if (proj.x > 0.01 && proj.x < 0.99 && proj.y > 0.01 && proj.y < 0.99 && proj.z < 1.0)
		return 1.0 - _surf_shadow_pcf(proj) * l.harshness;
	
	return 1.0;
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
		shadow_factor = _surf_shadow_factor(l, p);
		
	// TODO only affect "diffuse"
	shadow_factor *= (1-ambient_occlusion);

	
        // calculate per-light radiance
        vec3 radiance = l.color.rgb * _surf_brightness(l, p) * PI * shadow_factor;
        
        vec3 V = -view_dir;
        vec3 L = -_surf_light_dir(l, p);
        
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
		color.rgb += _surf_light_add(light[i], p, n, albedo.rgb, metal, roughness, ambient_occlusion, view_dir, i == shadow_index).rgb;
	
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
