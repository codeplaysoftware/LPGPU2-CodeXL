uniform sampler2D surface;

varying mediump vec2 texCoord;

uniform highp float phase;
uniform highp float A;
uniform highp float B;

void main() {
 mediump float x = texCoord.x + phase;

 highp float y = texCoord.y; y=y*(A*y+B);

 mediump vec2 tex = vec2(x,y);

 mediump vec4 color = texture2D(surface,tex);

 color.a = color.r < 0.5 ? 0.0 : 0.2;

 gl_FragColor = color;
}
