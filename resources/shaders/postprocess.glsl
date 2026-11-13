#VERTEX
void main() {
    gl_Position = vec4(v_position, 1.0);
}

#FRAGMENT

vec4 albedo(vec2 uv) {
    vec4 albedo_texture = (u_material.has_albedo_tx) ? texture(u_albedo_tx, uv) : vec4(1.0f);
    return albedo_texture * u_material.albedo_fac;
}

void main() {
    fragCol = albedo(v_uv) + vec4(v_position, 1);
}