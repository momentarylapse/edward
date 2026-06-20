<Layout>
	version = 420
	name = fog
</Layout>
<Module>


void fog_apply(inout vec4 color, vec3 p) {
	if (!fog_enabled)
		return;

	float distance = length(p - eye_pos.xyz);
	float f = exp(-distance * fog_density);
	color.rgb = f * color.rgb + (1-f) * fog_color.rgb;
}

</Module>
