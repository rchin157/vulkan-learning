#version 450

layout(location = 0) out vec4 outColour;

layout(push_constant) uniform Push {
    mat2 transform;
    vec2 offset;
    vec3 color;
} push;

void main() {
    outColour = vec4(push.color, 1.0);
}