<Layout>
	name = vertex-default
</Layout>
<Module>

struct Matrix {
	mat4 model, view, project;
};
/*layout(binding=0)*/ uniform Matrix matrix;

layout(location=0) in vec3 in_position;
layout(location=1) in vec3 in_normal;
layout(location=2) in vec2 in_uv;

layout(location=0) out vec4 out_pos; // view space
layout(location=1) out vec3 out_normal;
layout(location=2) out vec2 out_uv;

void main() {
	gl_Position = matrix.project * matrix.view * matrix.model * vec4(in_position, 1);
	out_pos = matrix.view * matrix.model * vec4(in_position, 1);
	out_normal =  (matrix.view * matrix.model * vec4(in_normal, 0)).xyz;
	out_uv = in_uv;
}

</Module>
