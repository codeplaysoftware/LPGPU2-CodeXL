uniform sampler2D surface0;
uniform sampler2D surface1;

uniform highp float interpolation;

varying mediump vec2  texCoord;


#define M_PI 3.1415926535897932

void main() {
 mediump float u = sin(2.0*M_PI*texCoord.x+interpolation);

 u = (u*(3.0-u*u)/2.0+1.0)/2.0;

 gl_FragColor = texture2D(surface0 ,texCoord) *        u + 
                texture2D(surface1 ,texCoord) * (1.0 - u);
}
