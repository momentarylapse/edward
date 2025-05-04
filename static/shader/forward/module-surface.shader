<Layout>
	version = 420
	name = surface-forward
</Layout>
<Module>

#import basic-data
#import lighting


// n - view space
void surface_out(vec3 n, vec4 albedo, vec4 emission, float metal, float roughness) {
	//out_color = emission;
	vec3 p = in_pos.xyz / in_pos.w;
	vec3 view_dir = normalize(p - eye_pos.xyz);
	

	out_color = perform_lighting(p, n, albedo, emission, metal, roughness, 0, eye_pos);
	
/*	float distance = length(p - eye_pos.xyz);
	float f = exp(-distance / fog.distance);
	out_color.rgb = f * out_color.rgb + (1-f) * fog.color.rgb;
	
	*/
	out_color.a = albedo.a;
}

// fabric shine through pass (against the light)
// transmissivity: per unit thickness
// layers: how many absorbing layers?
void surface_scattered_out(vec3 n, vec4 transmissivity, float layers) {
	surface_out(-n, pow(transmissivity, vec4(layers)), vec4(0), 0, 1);
}


// through a layer of constant thickness, seen from an angle
//  (ignoring off-center skewing for now...)
// transmissivity.rgb: color filter
//    - black => full transmission
//    - white => no transmission
// transmissivity.a:   filter "density"
//    - 0   => no filter, full transmission
//    - 0.5 => some filter/transmission
//    - 1   => full filter, no transmission
vec3 surface_transmissivity_factor(vec3 n, vec4 transmissivity) {
	// a=0 => T=white (full transmission)
	// a=0.5 => T=transmission...
	// a=1 => T=black (no transmission)
	vec3 T = pow(transmissivity.rgb, vec3(1/(1-transmissivity.a)-1));
	
	T = pow(T, vec3(1 / abs(n.z)));
	return T;
}


void surface_transmissivity_out(vec3 n, vec4 transmissivity) {
	out_color = vec4(surface_transmissivity_factor(n, transmissivity), 1);
}


void surface_reflectivity_out(vec3 n, vec4 albedo, vec4 emission, float metal, float roughness, vec4 transmissivity) {
	if (!gl_FrontFacing)
		n = - n;
	surface_out(n, albedo, emission, metal, roughness);
	out_color.a = 1;

	// reduced reflectivity
	out_color.rgb *= 1 - surface_transmissivity_factor(n, transmissivity);
//	out_color *= 1 - transmissivity * pow(abs(n.z), 2);
}

</Module>
