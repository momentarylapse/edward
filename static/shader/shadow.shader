<Layout>
	version = 450
	bindings = [[buffer,buffer,sampler,sampler,sampler,sampler,sampler]]
	pushsize = 0
	topology = triangles
</Layout>
<FragmentShader>

layout(location = 0) out vec4 out_color;

void main() {
	out_color = vec4(1,0,0,1);
}
</FragmentShader>
