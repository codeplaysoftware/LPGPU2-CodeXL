uniform sampler2D surface0;
uniform sampler2D surface1;

uniform mediump float interpolation;

varying mediump vec2  texCoord;


void main() {
 mediump float u = interpolation - floor(interpolation);

 gl_FragColor = texture2D(surface0 ,texCoord) *        u + 
                texture2D(surface1 ,texCoord) * (1.0 - u);
}
