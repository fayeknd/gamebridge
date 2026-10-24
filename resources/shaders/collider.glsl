#VERTEX
void main() {
    gl_Position = u_vp_matrix * u_model_matrix * vec4(v_position, 1.0);
}

#FRAGMENT

uniform vec4 u_box_colour;

void main() {
    fragCol = u_box_colour;
}