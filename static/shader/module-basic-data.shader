<Layout>
	version = 420
	name = basic-data
</Layout>
<Module>

#import basic-interface

layout(location = 0) in vec4 in_pos; // view space
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_uv;
layout(location = 3) in vec4 in_color;
layout(location = 0) out vec4 out_color;

</Module>
