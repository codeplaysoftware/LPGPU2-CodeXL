#version 400

#extension GL_ARB_separate_shader_objects  : enable
#extension GL_ARB_shading_language_420pack : enable

layout (std140, binding = 1) uniform buf2 {
 float phase;
} ubuf2;

layout (binding = 2) uniform sampler2D tex1;
layout (binding = 3) uniform sampler2D tex2;

layout (location = 0) in  vec2 texcoord;
layout (location = 0) out vec4 outColor;

void main() {

// Alternative colouring...
//  outColor = textureLod(tex1, texcoord, 0.0) * vec4(ubuf2.phase, 1.0, 1.0, 1.0);

 outColor = (textureLod(tex1, texcoord, 0.0)
  + textureLod(tex2, texcoord, 0.0)) * 0.5 * vec4(ubuf2.phase, 1.0, 1.0, 1.0);
}

