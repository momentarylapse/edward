<Layout>
	version = 420
	name = light-sources
</Layout>
<Module>


vec3 _light_source_dir(Light l, vec3 p) {
	if (l.radius < 0)
		return l.dir.xyz;
	return normalize(p - l.pos.xyz);
}


float _light_source_brightness(Light l, vec3 p) {
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
	return b * (1 - smoothstep(tmax*0.8, tmax, t)) * mix(1.0, pow(cos(pow(t/tmax, 3.7) * PI * 4), 2), 0.7*pow(cos(t/tmax * PI * 1), 2));
}

bool _light_source_reachable(Light l, vec3 p) {
	if (l.radius < 0)
		return true;
	if (length(p - l.pos.xyz) > l.radius*1.0)
		return false;
	return true;
}

</Module>
