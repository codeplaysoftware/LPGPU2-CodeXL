#version 400

#extension GL_ARB_separate_shader_objects  : enable
#extension GL_ARB_shading_language_420pack : enable

layout (std140, binding = 1) uniform frag_buf {
 int xres;
 int yres;
} uni;

layout (location = 0) in float col;

void main() {
 float c = 1.0 - gl_FragCoord.z;

 gl_FragColor = vec4(c, col, 1.0-col, 0.5);
}
