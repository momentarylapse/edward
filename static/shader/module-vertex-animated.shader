<Layout>
	name = vertex-animated
</Layout>
<Module>

struct Matrix {
	mat4 model, view, project;
};
/*layout(binding=0)*/ uniform Matrix matrix;

struct BoneData {
	mat4 matrix[128];
};
uniform BoneData bones;

layout(location=0) in vec3 in_position;
layout(location=1) in vec3 in_normal;
layout(location=2) in vec2 in_uv;
layout(location=3) in ivec4 in_bone_index;
layout(location=4) in vec4 in_bone_weight;

layout(location=0) out vec4 out_pos; // view space
layout(location=1) out vec3 out_normal;
layout(location=2) out vec2 out_uv;
layout(location=3) out vec4 out_color;

void main() {
	mat4 model = bones.matrix[in_bone_index.x] * in_bone_weight.x;
	model += bones.matrix[in_bone_index.y] * in_bone_weight.y;
	model += bones.matrix[in_bone_index.z] * in_bone_weight.z;
	model += bones.matrix[in_bone_index.w] * in_bone_weight.w;
	
	gl_Position = matrix.project * matrix.view * model * vec4(in_position, 1);
	out_pos = matrix.view * model * vec4(in_position, 1);
	out_normal = (matrix.view * model * vec4(in_normal, 0)).xyz;
	out_uv = in_uv;
	out_color = vec4(0,0,0,1);
}

</Module>
