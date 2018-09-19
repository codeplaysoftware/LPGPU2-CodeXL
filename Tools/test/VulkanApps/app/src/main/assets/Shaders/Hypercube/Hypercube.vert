#version 400

#extension GL_ARB_separate_shader_objects  : enable
#extension GL_ARB_shading_language_420pack : enable

layout (std140, binding = 0) uniform vert_buf {
 mat4  mvp;
 float phase;

 int   xres;
 int   yres;
} uni;

layout (location = 0) in  vec4  pos;

layout (location = 0) out float col;

vec4 rotate(vec4 v, float a, int i, int j) {
 float s = sin(a);
 float c = cos(a);

 float x = v[j-1]*s+v[i-1]*c;
 float y = v[j-1]*c-v[i-1]*s;

 v[i-1]=x;
 v[j-1]=y;

 return v;
}

const float scale = 16.0;

void main() {
 vec2 resolution = vec2(uni.xres+1,uni.yres+1);

 vec4 p4 = pos;

 float a = scale * uni.phase;

 p4 = rotate(p4,    a+     sin(0.25+a),1,4);
 p4 = rotate(p4,0.3*a+10.0*sin(0.02*a),2,4);
 p4 = rotate(p4,0.6*a                 ,3,4);

 col = p4.w;

 vec3 p3 = p4.xyz/(2.0+p4.w);

 gl_Position = uni.mvp * vec4(p3.x*resolution.y/resolution.x,p3.yz,1.0);
}

