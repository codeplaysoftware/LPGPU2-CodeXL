#version 400

#extension GL_ARB_separate_shader_objects  : enable
#extension GL_ARB_shading_language_420pack : enable

layout (std140, binding = 0) uniform vert_buf {
 mat4  mvp;
 float phase;
} uni;

layout (location = 0) in  vec4 pos;
layout (location = 1) in  vec2 tex;
layout (location = 0) out vec2 texcoord;

void main() {
 texcoord = tex;

 gl_Position = uni.mvp * pos;
}

