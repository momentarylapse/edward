<Layout>
	name = geometry-points
</Layout>
<Module>

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

layout(location=0) in vec4 in_pos[];
layout(location=1) in vec3 in_n[];
layout(location=2) in vec2 in_uv[];
layout(location=3) in vec4 in_color[];

layout(location=0) out vec4 out_pos;
layout(location=1) out vec3 out_n;
layout(location=2) out vec2 out_uv;
layout(location=3) out vec4 out_color;

#ifdef vulkan
const vec4 source_uv = vec4(0,1,0,1);
#else
uniform float target_width = 1920, target_height = 1080;
uniform float line_width = 4;
uniform vec4 source_uv = vec4(0,1,0,1);
#endif

void main() {
	vec4 p0 = gl_in[0].gl_Position;
        //float w0 = gl_in[0].gl_Position.w;
        //p0 /= 2.0;
        /*float w1 = gl_in[1].gl_Position.w;
        vec2 d = (gl_in[1].gl_Position.xy / w1 - gl_in[0].gl_Position.xy / w0);
        d = vec2(d.x * target_width/2, d.y * target_height/2);
        d = vec2(d.y, -d.x) / length(d) * line_width/2;
        d = vec2(d.x / target_width*2, d.y / target_height*2);*/
        
        // implicit perspective scaling! (p0 is un-normalized!)
	//float dx = in_r[0] * (target_height / target_width); //0.6;
	float in_r = in_uv[0].x;
	float dx = in_r * 0.6;
	float dy = in_r;
	// TODO correct aspect scaling!

        gl_Position = p0 + vec4(-dx,-dy,0,0);
        out_pos = gl_Position;
	out_n = vec3(0,0,-1);
        out_uv = vec2(source_uv[0], source_uv[2]);
        out_color = in_color[0];
        EmitVertex();

        gl_Position = p0 + vec4(dx,-dy,0,0);
        out_pos = gl_Position;
	out_n = vec3(0,0,-1);
        out_uv = vec2(source_uv[1], source_uv[2]);
        out_color = in_color[0];
        EmitVertex();
    
        gl_Position = p0 + vec4(-dx,dy,0,0);
        out_pos = gl_Position;
	out_n = vec3(0,0,-1);
        out_uv = vec2(source_uv[0], source_uv[3]);
        out_color = in_color[0];
        EmitVertex();

        gl_Position = p0 + vec4(dx,dy,0,0);
        out_pos = gl_Position;
	out_n = vec3(0,0,-1);
        out_uv = vec2(source_uv[1], source_uv[3]);
        out_color = in_color[0];
        EmitVertex();
    
        EndPrimitive();
}

</Module>

