precision mediump float;

uniform float phase;   // animation time variable
uniform float opacity; // opacity

uniform int   xres;    // screen X resolution
uniform int   yres;    // screen Y resolution



void main() // FRAG_OVERDRAW
{
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

  c= c - floor(c);

  v = vec4(c,c,c, opacity);
 }

 gl_FragColor = v;
}
