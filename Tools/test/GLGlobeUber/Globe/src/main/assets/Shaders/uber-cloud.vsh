attribute highp vec4  Position;
attribute highp vec3  Normal;
attribute highp vec2  Tex;

varying mediump vec2  texCoord;

uniform   highp mat4  uMVMatrix;
uniform   highp mat4  uPMatrix;

uniform   highp float hcphase; // hypercube phase


vec4 rotate(vec4 v, highp float a, int i, int j) {
 highp float s = sin(a);
 highp float c = cos(a);

 float x = v[j]*s+v[i]*c;
 float y = v[j]*c-v[i]*s;

 v[i] = x;
 v[j] = y;

 return v;
}


#define delta 0.05
#define M_PI 3.14159
#define EPS 0.1


void main(void) {
 highp vec4 p4 = Position; // vec4(Position.xyz,1);

 highp float a1 = 1.5 * sin(hcphase * 0.4);
 highp float a2 = 1.6 * sin(hcphase * 0.8);
 highp float a3 = 1.7 * sin(hcphase * 1.7);

 p4 = rotate(p4,5.821 * delta * a1,2,3);
 p4 = rotate(p4,8.0   * delta * a2,1,3);
 p4 = rotate(p4,M_PI  * delta * a3,0,3);

// p4 = rotate(p4,hcphase,0,3);

 vec3 p3 = p4.xyz/(1.0+EPS+p4.w/60.0);

 gl_Position = uPMatrix * vec4(p3,1.0);

 texCoord = Tex;
}
