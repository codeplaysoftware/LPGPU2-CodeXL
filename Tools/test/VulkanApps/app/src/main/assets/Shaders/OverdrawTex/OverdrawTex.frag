#version 400

#extension GL_ARB_separate_shader_objects  : enable
#extension GL_ARB_shading_language_420pack : enable

layout (std140, binding = 1) uniform buf2 {
 float phase;
 float opacity;
} uni;

layout (binding = 2) uniform sampler2D tex;

layout (location = 0) in  vec2 texcoord;
layout (location = 0) out vec4 outColor;

void main() {
 outColor = vec4(textureLod(tex, fract(texcoord), 0.0).xyz,uni.opacity);
}

