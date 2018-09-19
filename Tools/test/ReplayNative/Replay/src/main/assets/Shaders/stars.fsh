uniform sampler2D surface;

varying mediump vec2 texCoord;


void main() {
 gl_FragColor = texture2D(surface,texCoord);
}
