<Layout>
	name = vertex-lines
</Layout>
<Module>
#import basic-interface

layout(location = 0) in vec3 in_pos;
layout(location = 1) in float in_r;
layout(location = 2) in vec4 in_col;

layout(location = 0) out vec4 out_pos; // view space
layout(location = 1) out vec3 out_n; // n.x = r
layout(location = 2) out vec2 out_uv;
layout(location = 3) out vec4 out_col;

void main() {
	gl_Position = matrix.project * matrix.view * matrix.model * vec4(in_pos, 1);
	out_n.x = in_r;
	out_col = in_col;
}

</Module>
