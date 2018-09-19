/*
 * The MIT License
 *
 * Copyright (c) 2016-17 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */
class Menger: public AppRenderer {
 std::vector<float> vtx;

 int    mengerGeom;
 int    mengerFrag;

 int    nvtx;
 int    stride;

 double scale;
 int    loop;
 int    scene;
 double delta;

 GLuint frameLocation;
 double frame;

 GLuint screenXLocation;
 GLuint screenYLocation;

 int    ScreenXRes;
 int    ScreenYRes;

 GLuint   vertexShader;
 GLuint fragmentShader;
 GLuint tProgram;

 GLuint mengerFragLocation;

 void nxtvtx() {nvtx++;}

 void addvtx(float x, float y, float z) {
  vtx.resize(stride*(nvtx+1));

  vtx[stride*nvtx+1-1] = x;
  vtx[stride*nvtx+2-1] = y;
  vtx[stride*nvtx+3-1] = z;

// Getting ready for attributes - colour, texcoords, etc
  for(int i=4;i<=stride;i++) vtx[stride*nvtx+i-1] = 0.0;
 }

 void quad(double p1, double p2, double p3,
           double u1, double u2, double u3,
           double v1, double v2, double v3, double su, double sv)
 {
  addvtx(p1            ,p2            ,p3            ); nxtvtx();
  addvtx(p1+su*u1      ,p2+su*u2      ,p3+su*u3      ); nxtvtx();
  addvtx(p1      +sv*v1,p2      +sv*v2,p3      +sv*v3); nxtvtx();

  addvtx(p1      +sv*v1,p2      +sv*v2,p3      +sv*v3); nxtvtx();
  addvtx(p1+su*u1      ,p2+su*u2      ,p3+su*u3      ); nxtvtx();
  addvtx(p1+su*u1+sv*v1,p2+su*u2+sv*v2,p3+su*u3+sv*v3); nxtvtx();
 }

 void cube(double p1, double p2, double p3, double s) {
  p1-=0.5*s; p2-=0.5*s; p3-=0.5*s;

  quad(p1,p2,p3, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0,s,s);
  quad(p1,p2,p3, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0,s,s);
  quad(p1,p2,p3, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0,s,s);

  p1+=1.0*s; p2+=1.0*s; p3+=1.0*s;

  quad(p1,p2,p3,-1.0, 0.0, 0.0, 0.0,-1.0, 0.0,s,s);
  quad(p1,p2,p3, 0.0,-1.0, 0.0, 0.0, 0.0,-1.0,s,s);
  quad(p1,p2,p3, 0.0, 0.0,-1.0,-1.0, 0.0, 0.0,s,s);
 }

 void mengersheets(double p1, double p2, double p3,
                   double u1, double u2, double u3,
                   double v1, double v2, double v3,
                   double w1, double w2, double w3, int n)
 {
  if (!n)
   quad(p1,p2,p3,u1,u2,u3,v1,v2,v3,1.0,1.0);
  else
   for(int i=0;i<=2;i++)
    mengersheets(p1+w1*i/3.0,p2+w2*i/3.0,p3+w3*i/3.0,u1,u2,u3,v1,v2,v3,w1/3.0,w2/3.0,w3/3.0,n-1);
 }

 void menger(double s, int n) {
  double p1=-s/2,p2=-s/2,p3=-s/2;

  double u1=  s,u2=0.0,u3=0.0;
  double v1=0.0,v2=  s,v3=0.0;
  double w1=0.0,w2=0.0,w3=  s;

  mengersheets(p1,p2,p3,u1,u2,u3,v1,v2,v3,w1,w2,w3,n); // Generate a stack of sheets recursively in Z direction
  mengersheets(p1,p2,p3,v1,v2,v3,w1,w2,w3,u1,u2,u3,n); // Generate a stack of sheets recursively in Y direction
  mengersheets(p1,p2,p3,w1,w2,w3,u1,u2,u3,v1,v2,v3,n); // Generate a stack of sheets recursively in X direction

  quad(p1+s,p2  ,p3  ,         v1,v2,v3,w1,w2,w3,1.0,1.0);
  quad(p1  ,p2+s,p3  ,u1,u2,u3,         w1,w2,w3,1.0,1.0);
  quad(p1  ,p2  ,p3+s,u1,u2,u3,v1,v2,v3,         1.0,1.0);
 }

public:
 void ReplayInit() {
#if 1
// The 'correct' value of mengerGeom is mengerFrag+1, however, you might not be able
// to afford that much geometry, because the number of sheets involve is 3 * (1 + 3 ^ mengerGeom)
   mengerGeom = 2;

// Menger recursion level for the holes in the sponge
   mengerFrag = 5;

// Vertex array
   vtx.clear();

// Vertex count
   nvtx = 0;

// Number of float elements per vertex
   stride = 3;

// Scaling parameter controling the size of the cube
   scale = 1.0;


   menger(scale,mengerGeom);


// The loop variable increments every frame but gets reset to 0 after so many frame
// look in the render script, renderRubik.lua for details
   loop = 0;


// number of frames in a 'scene' - a larger slows the animation down
   scene = 50;


// 90 degress must take 'scene' frames. The value delta is the correct increment to achieve this
   delta = 0.5 * M_PI / scene;


// Load the shaders
      vertexShader = LoadShaderFromFile(GL_VERTEX_SHADER  , "menger.vert");
    fragmentShader = LoadShaderFromFile(GL_FRAGMENT_SHADER, "menger.frag");

// Create the program object
    tProgram = glCreateProgram();

// Attach shaders
    glAttachShader(tProgram,   vertexShader);
    glAttachShader(tProgram, fragmentShader);

// Bind vPosition to attribute 0
    glBindAttribLocation(tProgram, 0, "vPosition");

// Bind vColor to attribute 1
    glBindAttribLocation(tProgram, 1, "vColor");

// Link the program
    glLinkProgram(tProgram);

// Animation timing variable
    frameLocation = glGetUniformLocation(tProgram,"frame");
    frame = 0.0;

// Screen Resolution
    screenXLocation = glGetUniformLocation(tProgram,"xres");
    screenYLocation = glGetUniformLocation(tProgram,"yres");

    ScreenXRes = GetScreenWidth ();
    ScreenYRes = GetScreenHeight();

// Menger recursion level for fragment shader
    mengerFragLocation = glGetUniformLocation(tProgram,"level");

    glEnable(GL_DEPTH_TEST);
#endif
 }

 void ReplayRender(double tim) {
    glClearColor(1.0, 1.0, 1.0, 1.0);

// Clear the color buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

// Use the program object
    glUseProgram(tProgram);

// Set the animation variable
    glUniform1f(frameLocation,frame);
    frame = frame + 1.0;

// Menger Sponge recursion level
    glUniform1i(mengerFragLocation,mengerFrag);

// Set Screen Resolution uniforms
    glUniform1i(screenXLocation,ScreenXRes);
    glUniform1i(screenYLocation,ScreenYRes);

// Load the vertex data
    glVertexAttribPointer(0, stride, GL_FLOAT, GL_FALSE, stride * 4, vtx.data());
    glEnableVertexAttribArray(0);

    glDrawArrays(GL_TRIANGLES, 0, nvtx);

// Move to next frame and reset loop frame count of necessary
    if (loop > 7*scene) loop = 0;

    loop++;
 }
}; // Menger
