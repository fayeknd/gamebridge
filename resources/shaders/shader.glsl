#VERTEX
void main() {
    gl_Position = vp_matrix * model_matrix * vec4(v_position, 1.0);
}

#FRAGMENT
void main() {
    fragCol = vec4(v_position, 1.0);
}