attribute highp vec3 Position;
attribute highp vec3 Normal;
attribute highp vec2 Tex;

varying mediump vec2 texCoord;

uniform   highp mat4 uMVMatrix;
uniform   highp mat4 uPMatrix;


void main(void) {
 highp vec4 p = vec4(Position,1);

 gl_Position = uPMatrix * p;

 texCoord = Tex;
}
