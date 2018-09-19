#version 400

#extension GL_ARB_separate_shader_objects  : enable
#extension GL_ARB_shading_language_420pack : enable

layout (std140, binding = 1) uniform frag_buf {
 float phase;
} uni;

layout (binding = <?>) uniform sampler2D tex1;
layout (binding = <?>) uniform sampler2D tex2;

layout (location = 0) in  vec2 texcoord;
layout (location = 0) out vec4 outColor;

void main() {
 float a = 0.5+0.5*sin(10.0 * uni.phase);

 outColor =    a  * textureLod(tex1, texcoord, 0.0)
          + (1-a) * textureLod(tex2, texcoord, 0.0);
}

