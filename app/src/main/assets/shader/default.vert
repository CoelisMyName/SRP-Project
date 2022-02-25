#version 300 es
layout(location=0) in vec3 aPosition;
out vec4 vColor;
void main() {
    vColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
    gl_Position = vec4(aPosition.x, aPosition.y, aPosition.z, 1.0f);
}