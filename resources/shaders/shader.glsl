#VERTEX
void main() {
    gl_Position = vec4(v_position, 1);
}

#FRAGMENT
void main() {
    fragCol = vec4(v_position, 1);
}