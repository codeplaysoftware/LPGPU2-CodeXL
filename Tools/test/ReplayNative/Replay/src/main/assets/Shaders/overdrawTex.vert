attribute vec4 vPosition;
attribute vec2 vTex;

uniform   float phase;

varying   vec2 texcoord;

void main() { // VERT_TEXOVERDRAW
 gl_Position = vPosition;

 texcoord = vec2(vTex.x-phase,1.0-vTex.y);
}
