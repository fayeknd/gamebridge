#VERTEX
void main() {
    gl_Position = vp_matrix * model_matrix * vec4(v_position, 1.0);
}

#FRAGMENT
uniform sampler2D tx;

void main() {
    fragCol = texture(tx, v_uv);
}