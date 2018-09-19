#version 400

#extension GL_ARB_separate_shader_objects  : enable
#extension GL_ARB_shading_language_420pack : enable

layout (std140, binding = 1) uniform frag_buf {
 float offset;
 float A;
 float B;
} uni;

layout (binding = <?>) uniform sampler2D tex;

layout (location = 0) in  vec2 texcoord;
layout (location = 0) out vec4 outColor;

void main() {
 vec2 t = vec2(fract(texcoord.x+uni.offset),texcoord.y);

 vec4 col = textureLod(tex, t, 0.0);

 if (col.r < 0.5) discard; else col.a = 0.4;

 outColor = col;
}

