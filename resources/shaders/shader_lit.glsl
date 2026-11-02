#VERTEX
void main() {
    gl_Position = u_vp_matrix * u_model_matrix * vec4(v_position, 1.0);
}

#FRAGMENT

vec4 albedo(vec2 uv) {
    vec4 albedo_texture = (u_material.has_albedo_tx) ? texture(u_albedo_tx, uv) : vec4(1.0f);
    return albedo_texture * u_material.albedo_fac;
}

float depth() {
    float near = 0.1; 
    float far  = 100.0; 

    float z = gl_FragCoord.z * 2.0 - 1.0; 
    return (2.0 * near * far) / (far + near - z * (far - near)) / far;
}

void main() {
    //fragCol = albedo(v_uv) * vec4(0.5 + (v_normal * 0.5) , 1);
    //fragCol = vec4(0.5 + (v_normal * 0.5) , 1);
    //fragCol = vec4(vec3(depth()), 1);
    fragCol = albedo(v_uv) * v_colour;
}