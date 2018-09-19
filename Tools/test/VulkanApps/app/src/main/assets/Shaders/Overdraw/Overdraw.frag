#version 400

#extension GL_ARB_separate_shader_objects  : enable
#extension GL_ARB_shading_language_420pack : enable

layout (std140, binding = 1) uniform buf2 {
 float phase;
 float opacity;
 int   xres;
 int   yres;
} uni;


float phase   = uni.phase;   // animation time variable
float opacity = uni.opacity; // opacity

int   xres    = uni.xres;    // screen X resolution
int   yres    = uni.yres;    // screen Y resolution

void main() {
 vec2 resolution = vec2(xres,yres);

 float width = resolution.x;
 float sep1 =     width/3.0;
 float sep2 = 2.0*width/3.0;

 vec4 v;

 if (gl_FragCoord.x < sep1) {
  // The left stripe is for observing the effects of opacity unimpeded

  v = vec4(1.0, 0.0, 0.0, opacity);
 } else if (gl_FragCoord.x < sep2) {
  // This middle stripe is for visually confirming the draw order - use opacity = 1.0

  v = vec4(0.0, 1.0 - 2.0 * gl_FragCoord.z, 0.0, opacity);
 } else {
  // The right-most stripe is for visually confirming the frame-rate - i.e is something moving?

  float c = phase - gl_FragCoord.z;

  c = c - floor(c);

  v = vec4(c,c,c, opacity);
 }

 gl_FragColor = v;
}
