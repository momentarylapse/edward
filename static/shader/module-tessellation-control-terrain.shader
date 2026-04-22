<Layout>
	name = tessellation-control-terrain
</Layout>
<Module>

layout(vertices = 4) out;

layout(location = 0) in vec4 in_pos[]; // view space
layout(location = 1) in vec3 in_normal[];
layout(location = 2) in vec2 in_uv[];
layout(location = 3) in vec4 in_color[];

layout(location = 0) out vec4 out_pos[4];
layout(location = 1) out vec3 out_normal[4];
layout(location = 2) out vec2 out_uv[4];
layout(location = 3) out vec4 out_color[4];

void main(void) {
	if (gl_InvocationID == 0) {
#if 0
		gl_TessLevelInner[0] = 16;
		gl_TessLevelInner[1] = 16;
		gl_TessLevelOuter[0] = 16;
		gl_TessLevelOuter[1] = 16;
		gl_TessLevelOuter[2] = 16;
		gl_TessLevelOuter[3] = 16;
#endif
		const int MIN_TESS_LEVEL = 1;
		const int MAX_TESS_LEVEL = 32;
		const float L = length(gl_in[1].gl_Position - gl_in[0].gl_Position);
		const float MIN_DISTANCE = L;
		const float MAX_DISTANCE = L*20;
		
		vec4 eyeSpacePos00 = gl_in[0].gl_Position;
		vec4 eyeSpacePos01 = gl_in[1].gl_Position;
		vec4 eyeSpacePos10 = gl_in[2].gl_Position;
		vec4 eyeSpacePos11 = gl_in[3].gl_Position;
		
		float distance00 = clamp((abs(eyeSpacePos00.z)-MIN_DISTANCE) / (MAX_DISTANCE-MIN_DISTANCE), 0.0, 1.0);
		float distance01 = clamp((abs(eyeSpacePos01.z)-MIN_DISTANCE) / (MAX_DISTANCE-MIN_DISTANCE), 0.0, 1.0);
		float distance10 = clamp((abs(eyeSpacePos10.z)-MIN_DISTANCE) / (MAX_DISTANCE-MIN_DISTANCE), 0.0, 1.0);
		float distance11 = clamp((abs(eyeSpacePos11.z)-MIN_DISTANCE) / (MAX_DISTANCE-MIN_DISTANCE), 0.0, 1.0);
		
		float tessLevel0 = mix(MAX_TESS_LEVEL, MIN_TESS_LEVEL, pow(min(distance10, distance00), 0.3));
		float tessLevel1 = mix(MAX_TESS_LEVEL, MIN_TESS_LEVEL, pow(min(distance00, distance01), 0.3));
		float tessLevel2 = mix(MAX_TESS_LEVEL, MIN_TESS_LEVEL, pow(min(distance01, distance11), 0.3));
		float tessLevel3 = mix(MAX_TESS_LEVEL, MIN_TESS_LEVEL, pow(min(distance11, distance10), 0.3));
		
		gl_TessLevelOuter[0] = tessLevel0;
		gl_TessLevelOuter[1] = tessLevel1;
		gl_TessLevelOuter[2] = tessLevel2;
		gl_TessLevelOuter[3] = tessLevel3;
		
		gl_TessLevelInner[0] = max(tessLevel1, tessLevel3);
		gl_TessLevelInner[1] = max(tessLevel0, tessLevel2);
	}

	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
	out_pos[gl_InvocationID] = in_pos[gl_InvocationID];
	out_normal[gl_InvocationID] = in_normal[gl_InvocationID];
	out_uv[gl_InvocationID] = in_uv[gl_InvocationID];
	out_color[gl_InvocationID] = in_color[gl_InvocationID];
}

</Module>
