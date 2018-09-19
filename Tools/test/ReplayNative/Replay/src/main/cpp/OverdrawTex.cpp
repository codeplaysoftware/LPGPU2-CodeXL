/*
 * The MIT License
 *
 * Copyright (c) 2016-17 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */
// #define GOOD



class OverdrawTex: public AppRenderer {
 template <typename T> int sgn(T val) {return (T(0) < val) - (val < T(0));}

 const int stride = 5; // x,y,z,u,v

// Create points for the quad in array, vtx
 void quad(int i, int t) {
  const int s=stride;

  vtx.resize(s*3*(t+2));

  int p=3*t;

  coords(vtx[s*p+1-1],vtx[s*p+2-1],vtx[s*p+3-1],vtx[s*p+4-1],vtx[s*p+5-1],i,-1, 1); p++;
  coords(vtx[s*p+1-1],vtx[s*p+2-1],vtx[s*p+3-1],vtx[s*p+4-1],vtx[s*p+5-1],i,-1,-1); p++;
  coords(vtx[s*p+1-1],vtx[s*p+2-1],vtx[s*p+3-1],vtx[s*p+4-1],vtx[s*p+5-1],i, 1,-1); p++;

  coords(vtx[s*p+1-1],vtx[s*p+2-1],vtx[s*p+3-1],vtx[s*p+4-1],vtx[s*p+5-1],i,-1, 1); p++;
  coords(vtx[s*p+1-1],vtx[s*p+2-1],vtx[s*p+3-1],vtx[s*p+4-1],vtx[s*p+5-1],i, 1,-1); p++;
  coords(vtx[s*p+1-1],vtx[s*p+2-1],vtx[s*p+3-1],vtx[s*p+4-1],vtx[s*p+5-1],i, 1, 1); p++;
 }


 void coords(float& x, float& y, float& z, float& u, float& v, int i, double U, double V) {
  double t = -(i-1.0)/(num-1);

  z = t;
  x = U + 0.2 * t * sgn(U);
  y = V + 0.2 * t * sgn(V);

  u = (U+1)/2;
  v = (V+1)/2;
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

 GLuint VBO,VAO;

 GLuint texture;

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
      vertexShader = LoadShaderFromFile(GL_VERTEX_SHADER  , "overdrawTex.vert");
    fragmentShader = LoadShaderFromFile(GL_FRAGMENT_SHADER, "overdrawTex.frag");

// Create the program object
    tProgram = glCreateProgram();

// Attach shaders
    glAttachShader(tProgram, vertexShader);
    glAttachShader(tProgram, fragmentShader);

// Bind vPosition to attribute 0
//  glBindAttribLocation(tProgram, 0, "vPosition");

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

    glGenVertexArrays(1,&VAO);
    glGenBuffers     (1,&VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER,VBO);
    glBufferData(GL_ARRAY_BUFFER,vtx.size()*sizeof(float),vtx.data(),GL_STATIC_DRAW);

    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,stride*sizeof(GLfloat),(GLvoid*)(0));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,stride*sizeof(GLfloat),(GLvoid*)(3*sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER,0);
    glBindVertexArray(0);

    texture = LPGPU2::LoadTexture("Images/topo.png");

#ifdef GOOD
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,texture);
#endif
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
    phase = phase + 0.25/num; if (phase>1) phase--;

// Set the opacity variable
    glUniform1f(opacityLocation,opacity);

#ifndef GOOD
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,texture);
#endif
    glUniform1i(glGetUniformLocation(tProgram,"ourTexture"),0);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES,0,3*ntri);
    glBindVertexArray(0);
 }
}; // OverdrawTex
