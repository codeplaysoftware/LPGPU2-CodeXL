#version 400

#extension GL_ARB_separate_shader_objects  : enable
#extension GL_ARB_shading_language_420pack : enable

layout (std140, binding = 0) uniform vert_buf {
 mat4  mvp;
 float phase;
 float amp;
} uni;

layout (location = 0) in  vec4 pos;
layout (location = 1) in  vec2 tex;
layout (location = 0) out vec2 texcoord;

// const float amp = 1.0f;

void main() {


 float phase = uni.phase;
 float amp   = uni.amp;

 float scale = 5.0f;

 vec4 stretch = vec4(
  scale + amp * sin(phase),
  scale + amp * cos(phase),
  scale + amp * 0.5f * ( sin(phase*0.27f) + cos(phase*0.39f) ),
  scale
 );



 texcoord = vec2(tex.x,1.0f-tex.y);

 gl_Position = uni.mvp * (pos * stretch);
}

