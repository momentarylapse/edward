<Layout>
	version = 420
	name = shadows
</Layout>
<Module>

// import basic-data first!


// amount of shadow
float _shadow_sample_z(vec3 p, vec2 dd, ivec2 ts) {
	vec2 d = dd / ts * 0.8;
	vec2 tp = p.xy + d;
	float epsilon = 0.004;
	if (tp.x > 0.39 && tp.y > 0.39 && tp.x < 0.61 && tp.y < 0.61)
		return texture(tex_shadow0, (p.xy - vec2(0.5,0.5))*4 + vec2(0.5,0.5) + d).r + epsilon;
	if (tp.x > 0.05 && tp.y > 0.05 && tp.x < 0.95 && tp.y < 0.95)
		return texture(tex_shadow1, p.xy + d / 2).r + epsilon;
	return 1.0;
}

vec2 VogelDiskSample(int sampleIndex, int samplesCount, float phi) {
	float GoldenAngle = 2.4;

	float r = sqrt(sampleIndex + 0.5) / sqrt(samplesCount);
	float theta = sampleIndex * GoldenAngle + phi;
	return vec2(r * cos(theta), r * sin(theta));
}

float _shadow_pcf(vec3 p) {
	ivec2 ts = textureSize(tex_shadow0, 0);
	//float value = 0;//shadow_pcf_step(p, vec2(0,0), ts);
	const float R = 4;
	const int N = 6;
	float phi0 = _surf_rand3d(p) * 2 * 3.1415;
	//float phi0 = fract(p.x * 43327.32141) * 2 * 3.1415;
	float num_blocked = 0;
	for (int i=0; i<N; i++) {
		vec2 dd = VogelDiskSample(i, N, phi0) * R;
		float z = _shadow_sample_z(p, dd, ts);
		if (z < p.z)
			num_blocked += 1;
	}
	float fN = N;
	return num_blocked / fN;
}

vec3 _light_proj(Light l, vec3 p) {
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

float _shadow_factor(Light l, vec3 p) {
	vec3 proj = _light_proj(l, p);
	
	if (proj.x > 0.01 && proj.x < 0.99 && proj.y > 0.01 && proj.y < 0.99 && proj.z < 1.0)
		return 1.0 - _shadow_pcf(proj) * l.harshness;
	
	return 1.0;
}

</Module>
