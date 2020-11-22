<Layout>
	version = 420
	name = surface-simple
</Layout>
<Module>


uniform vec3 eye_pos;


struct Light {
	mat4 proj;
	vec4 pos;
	vec4 dir;
	vec4 color;
	float radius, theta, harshness;
};
uniform int num_lights;
//uniform int shadow_index = 0;

/*layout(binding = 1)*/ uniform LightData {
	Light light[32];
};

struct Fog {
	vec4 color;
	float distance;
};
/*layout(binding = 3)*/ uniform Fog fog;

uniform samplerCube tex4;


layout(location = 0) in vec4 in_pos; // world
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec3 in_normal;

layout(location = 0) out vec4 out_color;

const float PI = 3.141592654;

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}



float _surf_rand3d(vec3 p) {
	return fract(sin(dot(p ,vec3(12.9898,78.233,4213.1234))) * 43758.5453);
}

vec3 _surf_light_dir(Light l, vec3 p) {
	if (l.radius < 0)
		return l.dir.xyz;
	return normalize(p - l.pos.xyz);
}


float _surf_brightness(Light l, vec3 p, vec3 n) {
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
	
	float t = acos(dot(l.dir.xyz, normalize(p - l.pos.xyz)));
	float tmax = l.theta;
	return b * (1 - smoothstep(tmax*0.8, tmax, t));
}

// blinn phong
float _surf_specular(Light l, vec3 p, vec3 n, float roughness, vec3 view_dir) {
	vec3 half_dir = normalize(_surf_light_dir(l, p) + view_dir);

	float spec_angle = max(-dot(half_dir, n), 0.0);
	return pow(spec_angle, 30*(1-roughness));
}


vec3 _surf_light_add(Light l, vec3 p, vec3 n, vec3 albedo, float reflectivity, float roughness, vec3 view_dir) {
	float shadow = 1.0;
	
	
	
	vec3 F0 = vec3(0.04);
	F0 = mix(F0, albedo, reflectivity);
	
	
        // calculate per-light radiance
        vec3 V = -view_dir;
        vec3 L = -_surf_light_dir(l, p);
        vec3 H = normalize(V + L);
        vec3 radiance     = l.color.rgb * _surf_brightness(l, p, n)*PI;
        
        // cook-torrance brdf
        float NDF = DistributionGGX(n, H, roughness);
        float G   = GeometrySmith(n, V, L, roughness);
        vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);
        
        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - reflectivity;
        
        vec3 numerator    = NDF * G * F;
        float denominator = 4.0 * max(dot(n, V), 0.0) * max(dot(n, L), 0.0);
        vec3 specular     = numerator / max(denominator, 0.001);
            
        // add to outgoing radiance Lo
        float NdotL = max(dot(n, L), 0.0);
        return (kD * albedo / PI + specular) * radiance * NdotL;
        
        
        
        
	
	

	float b = _surf_brightness(l, p, n) * shadow;
	float lambert = max(-dot(n, _surf_light_dir(l, p)), 0);
	
	float bb = l.harshness * lambert + (1 - l.harshness);
	vec3 col = albedo * l.color.rgb * bb * b;
	
	// specular
	if (lambert > 0 && reflectivity > 0.01) {
		float spx = _surf_specular(l, p, n, roughness, view_dir);
		col = mix(col, spx * l.color.rgb * b, reflectivity);
	}
	return col;
}

void surface_out(vec3 n, vec4 albedo, vec4 emission, float reflectivity, float roughness) {
	out_color = emission;
	vec3 p = in_pos.xyz / in_pos.w;
	vec3 view_dir = normalize(p - eye_pos.xyz);
	
	roughness = max(roughness, 0.08);
	
///	float reflectivity = 1-((1-xxx.x) * (1-exp(-pow(dot(d, n),2) * 100)));

	if (reflectivity > 0.01) {
		vec3 L = reflect(view_dir, normalize(n));
		vec4 r = texture(tex4, L);
		/*if (roughness > 0.1) {
			r += texture(tex4, reflect(view_dir, normalize(n + vec3(_surf_rand3d(p),0,1) * roughness/10)));
			r += texture(tex4, reflect(view_dir, normalize(n + vec3(1,_surf_rand3d(p),0) * roughness/10)));
			r += texture(tex4, reflect(view_dir, normalize(n + vec3(0,1,_surf_rand3d(p)) * roughness/10)));
			r /= 5;
		}
		out_color += r * reflectivity;*/
		
		
		
		
		
	//L=-L;
	vec3 F0 = vec3(0.04);
	F0 = mix(F0, albedo.rgb, reflectivity);
	
	
        // calculate per-light radiance
        vec3 V = -view_dir;
        vec3 H = normalize(V + L);
        vec3 radiance     = r.rgb*PI;
        
        // cook-torrance brdf
        float NDF = DistributionGGX(n, H, roughness);
        float G   = GeometrySmith(n, V, L, roughness);
        vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);
        //out_color.rgb = F;
        //return;
        
        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - reflectivity;
        
        vec3 numerator    = NDF * G * F;
        float denominator = 4.0 * max(dot(n, V), 0.0) * max(dot(n, L), 0.0);
        vec3 specular     = numerator / max(denominator, 0.001);
            
        // add to outgoing radiance Lo
        float NdotL = max(dot(n, L), 0.0);
        out_color.rgb += (/*kD * albedo.rgb / PI +*/ specular) * radiance * NdotL/3000;
		
		
	}
	

	for (int i=0; i<num_lights; i++)
		out_color.rgb += _surf_light_add(light[i], p, n, albedo.rgb, reflectivity, roughness, view_dir).rgb;
	
/*	float distance = length(p - eye_pos.xyz);
	float f = exp(-distance / fog.distance);
	out_color.rgb = f * out_color.rgb + (1-f) * fog.color.rgb;
	
	*/
	
	out_color.a = albedo.a;
}
</Module>
