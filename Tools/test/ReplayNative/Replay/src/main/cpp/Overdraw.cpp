/*
 * The MIT License
 *
 * Copyright (c) 2016-17 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */
class Overdraw: public AppRenderer {
 template <typename T> int sgn(T val) {return (T(0) < val) - (val < T(0));}

// Create points for the quad in array, vtx
 void quad(int i,int t) {
  vtx.resize(9*(t+2));

  coords(vtx[9*t+1-1],vtx[9*t+2-1],vtx[9*t+3-1],i,-1, 1);
  coords(vtx[9*t+4-1],vtx[9*t+5-1],vtx[9*t+6-1],i,-1,-1);
  coords(vtx[9*t+7-1],vtx[9*t+8-1],vtx[9*t+9-1],i, 1,-1);

  t=t+1;

  coords(vtx[9*t+1-1],vtx[9*t+2-1],vtx[9*t+3-1],i,-1, 1);
  coords(vtx[9*t+4-1],vtx[9*t+5-1],vtx[9*t+6-1],i, 1,-1);
  coords(vtx[9*t+7-1],vtx[9*t+8-1],vtx[9*t+9-1],i, 1, 1);
 }


 void coords(float& u, float& v, float& c, int i, double x, double y) {
  double t = -(i-1.0)/(num-1);

  c = t;
  u = x + 0.2 * t * sgn(x);
  v = y + 0.2 * t * sgn(y);
 }

 int num;
 int ntri;
 double opacity;
 int order;
 std::vector<float> vtx;

 int from,to,step;

 GLuint tProgram;
 GLuint   vertexShader;
 GLuint fragmentShader;

 GLuint opacityLocation;

 GLuint phaseLocation;
 double  phase;

 GLuint screenXLocation;
 GLuint screenYLocation;

 int ScreenXRes;
 int ScreenYRes;

public:
 void ReplayInit() {
// Number of sheets in z
   num = 50;

// Opacity of the sheets
   opacity = 0.05;

// Draw order of the sheets
   order =  1; // back to front
// order = -1; // front to back

// The vertex array
   vtx.clear(); // = {}

// Triangle count
   ntri = 0;

// Set up the draw order for populating the vtx array based on the 'order' variable
   if (order > 0) {from=1;   to=num; step= 1;}
   else           {from=num; to=1;   step=-1;}

// With everything in place, populating the vertex array is trivial
   for(int i=from;i<=to;i+=step) {
    quad(i,ntri); ntri = ntri + 2;
   }

// Load the shaders
      vertexShader = LoadShaderFromFile(GL_VERTEX_SHADER  , "overdraw.vert");
    fragmentShader = LoadShaderFromFile(GL_FRAGMENT_SHADER, "overdraw.frag");


// Create the program object
    tProgram = glCreateProgram();

// Attach shaders
    glAttachShader(tProgram, vertexShader);
    glAttachShader(tProgram, fragmentShader);


// Bind vPosition to attribute 0
    glBindAttribLocation(tProgram, 0, "vPosition");

// Link the program
    glLinkProgram(tProgram);

// Animation timing variable
    phaseLocation = glGetUniformLocation(tProgram,"phase");
    phase = 0.0;

// Screen Resolution
    screenXLocation = glGetUniformLocation(tProgram,"xres");
    screenYLocation = glGetUniformLocation(tProgram,"yres");

    ScreenXRes = GetScreenWidth ();
    ScreenYRes = GetScreenHeight();

    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);

// Transparency variable
    opacityLocation = glGetUniformLocation(tProgram,"opacity");
 }

 void ReplayRender(double tim) {
    glClearColor(0.0, 0.0, 0.5, 1.0);

// Clear the color buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

// Use the program object
    glUseProgram(tProgram);

// Set the animation variable
    glUniform1f(phaseLocation,phase);

// Set Screen Resolution uniforms
    glUniform1i(screenXLocation,ScreenXRes);
    glUniform1i(screenYLocation,ScreenYRes);

// advance phase variable
//  if it is increased by 1.0/num the colouring will move one sheet per frame,
//  however, this makes it difficult to see movement with a large number of sheets,
//  incrementing by a number like pi means we will see movement and there is no chance of
//  temporal aliasing (advancing by an exact multiple of the number of sheets)
    phase = phase + 1.0/num; // math.pi

// Set the opacity variable
    glUniform1f(opacityLocation,opacity);

// Load the vertex data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vtx.data());
    glEnableVertexAttribArray(0);
    glDrawArrays(GL_TRIANGLES, 0, ntri * 3);
 }
}; // Overdraw
