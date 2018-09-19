#version 400

#extension GL_ARB_separate_shader_objects  : enable
#extension GL_ARB_shading_language_420pack : enable

layout (std140, binding = 1) uniform buf2 {
 float phase;
 int   xres;
 int   yres;
} uni;



float phase = uni.phase;
int   xres  = uni.xres;
int   yres  = uni.yres;



// Prerequisites...

const int SHAPE_CUBE       = 1;
const int SHAPE_SPHERE     = 2;
const int SHAPE_OCTAHEDRON = 3;



// Configuration...

const int   MAX_ITER = 100;   // Maximum number of iterations
const float MAX_DIST = 100.0; // Maximum ray distance before giving up
const float CAM_DIST =  10.0; // Camera distance

const float EPSILON  = 0.001; // The 'close enough' constant

const int NX = 5; // The grid of shapes is -NX .. NX in X
const int NY = 5; // The grid of shapes is -NY .. NY in Y
const int NZ = 4; // The grid of shapes is -NZ .. NZ in Z

const vec3 separation = vec3(2.8,2.8,2.8); // separation of objects in X,Y and Z


// Display constants (for visuals only - do not affect performance)

const float DISPLAY_DEPTH = 10.0; // All objects within this distance of the origin


// Derived constants...

const vec3 centre = vec3(float(NX-1)/2.0,float(NY-1)/2.0,float(NZ-1)/2.0);


// Distance to closest point of a sphere centred at the origin...
float sphere(vec3 pos, float radius) {
 return length(pos) - radius;
}

// Distance to closest point of a rectangular box centred at the origin...
float box(vec3 pos, vec3 siz) {
 return length(max(abs(pos) - siz, 0.0));
}

// Distance to closest point of a square cube centred at the origin...
float cube(vec3 pos, float siz) {
 return length(max(abs(pos) - siz, 0.0));
}

// Distance to closest point of an octahedron centred at the origin...
float octahedron(vec3 pos, float siz) {
 return (dot(abs(pos),vec3(1,1,1)) - siz)/sqrt(3.0);
}



float distfunc(vec3 origin, vec3 dir) {
 float total = 0.0;
 vec3 ray = origin;
 float dist = EPSILON;

 for(int i=0;i<MAX_ITER;i++) {
  if (dist < EPSILON || total > MAX_DIST)
   break;

  dist = cube(ray,1.0);

  total += dist;
  ray   += dist * dir;
 }

 return total;
}



float distfunc(vec3 origin, vec3 dir, int shape) {
 float total = 0.0;
 vec3 ray = origin;
 float dist = EPSILON;

 for(int i=0;i<MAX_ITER;i++) {
  if (dist < EPSILON || total > MAX_DIST)
   break;

  if      (shape == SHAPE_SPHERE    ) dist = sphere    (ray,1.0);
  else if (shape == SHAPE_CUBE      ) dist = cube      (ray,1.0);
  else if (shape == SHAPE_OCTAHEDRON) dist = octahedron(ray,1.0);

  total += dist;
  ray   += dist * dir;
 }

 return total;
}



void main() {
 vec2 resolution = vec2(xres,yres);

 vec2 pos = gl_FragCoord.xy / resolution;

 vec3 cameraOrigin = CAM_DIST * normalize(vec3(sin(phase), cos(2.0*phase), sin(3.0*phase)));
 vec3 cameraTarget = vec3(0.0, 0.0, 0.0);
 vec3 upDirection  = vec3(0.0, 1.0, 0.0);

 vec3 cameraDir    = normalize(cameraTarget - cameraOrigin);
 vec3 cameraRight  = normalize(cross(upDirection,cameraOrigin));
 vec3 cameraUp     = cross(cameraDir,cameraRight);

 vec2 screenPos = -1.0 + 2.0 * pos;

 screenPos.x *= resolution.x / resolution.y; // aspect correction

 vec3 rayDir = normalize(cameraRight * screenPos.x + cameraUp * screenPos.y + cameraDir);

 float dist = MAX_DIST;

 int shape = 1;

 for(int ix=0;ix<NX;ix++)
 for(int iy=0;iy<NY;iy++)
 for(int iz=0;iz<NZ;iz++) {
  vec3 rayOrigin = cameraOrigin+separation*(vec3(ix,iy,iz)-centre);
  dist = min(dist,distfunc(rayOrigin,rayDir,shape));

  // cube -> sphere -> octa -> cube -> sphere -> octa -> etc...
  shape += 1;
  if (shape>3) shape = 1;
 }

 if (dist >= MAX_DIST) {
  // Mid red for background if no intersection...
  gl_FragColor = vec4(0.5,0.0,0.0,1.0);
 } else {
  // Shade from white (closest) to blue (farthest)
  float shade = 1.0 - 0.5 * (dist + DISPLAY_DEPTH - CAM_DIST) / DISPLAY_DEPTH;
  gl_FragColor = vec4(shade,shade,1.0,1.0);
 }
}
