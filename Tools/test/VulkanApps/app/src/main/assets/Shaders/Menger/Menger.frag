#version 400

#extension GL_ARB_separate_shader_objects  : enable
#extension GL_ARB_shading_language_420pack : enable

layout (std140, binding = 1) uniform frag_buf {
 int level;
} uni;


layout (location = 0) in vec3 position;


const float epsilon = 0.004; // any smaller and you get visual artifacts

const float lo = 1.0/3.0;
const float hi = 1.0-lo;


int within(float p, float eps) {return p>=lo+eps && p<=hi-eps ? 1:0;}

int count(vec3 p, float eps) {return within(p.x,eps) + within(p.y,eps) + within(p.z,eps);}


bool inside(vec3 p) {
 float eps = epsilon;

 if (count(p,eps)>1) return true;

 for(int i=0;i<uni.level;i++) {
  p.x = 3.0 * p.x; while(p.x>1.0) p.x--;
  p.y = 3.0 * p.y; while(p.y>1.0) p.y--;
  p.z = 3.0 * p.z; while(p.z>1.0) p.z--;

  if (count(p,eps)>1) return true;

  eps *= 3.0;
 }

 return false;
}


void main() {
 float c = 1.0 - gl_FragCoord.z;

 if (inside(position)) discard;

 gl_FragColor = vec4(c,c,1.0,1.0);
}

