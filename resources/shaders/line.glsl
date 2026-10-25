#VERTEX
void main() {
    gl_Position = u_vp_matrix * vec4(v_position, 1.0);
}

#FRAGMENT
void main() {
    fragCol = v_colour;
}