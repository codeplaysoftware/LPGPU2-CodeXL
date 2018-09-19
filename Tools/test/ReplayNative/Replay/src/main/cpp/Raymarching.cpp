/*
 * The MIT License
 *
 * Copyright (c) 2016-17 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */
class Raymarching: public AppRenderer {
 GLuint   vertexShader;
 GLuint fragmentShader;

 GLuint tProgram;

 GLuint phaseLocation;
 double phase;

 GLuint screenXLocation;
 GLuint screenYLocation;

 int ScreenXRes;
 int ScreenYRes;

 float vtx[18] {
  -1.0,  1.0, 0.0, // tri 1
  -1.0, -1.0, 0.0,
   1.0, -1.0, 0.0,
   1.0, -1.0, 0.0, // tri 2
  -1.0,  1.0, 0.0,
   1.0,  1.0, 0.0
 };

public:
 void ReplayInit() {
    vertexShader = LoadShaderFromFile(GL_VERTEX_SHADER  , "raymarching.vert");
  fragmentShader = LoadShaderFromFile(GL_FRAGMENT_SHADER, "raymarching.frag");

  tProgram = glCreateProgram();

  glAttachShader(tProgram, vertexShader);
  glAttachShader(tProgram, fragmentShader);

  glBindAttribLocation(tProgram, 0, "vPosition");

  glLinkProgram(tProgram);

  phaseLocation = glGetUniformLocation(tProgram,"phase");
  phase = 0.25;

  screenXLocation = glGetUniformLocation(tProgram,"xres");
  screenYLocation = glGetUniformLocation(tProgram,"yres");

  ScreenXRes = GetScreenWidth ();
  ScreenYRes = GetScreenHeight();

  glClearColor(0.0, 0.0, 0.0, 1.0);
 }

 void ReplayRender(double tim) {
  glClear(GL_COLOR_BUFFER_BIT);

  glUseProgram(tProgram);

  glUniform1f(phaseLocation,phase);
  phase = phase + 0.01;

  glUniform1i(screenXLocation,ScreenXRes);
  glUniform1i(screenYLocation,ScreenYRes);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vtx);
  glEnableVertexAttribArray(0);
  glDrawArrays(GL_TRIANGLES, 0, 6);

//glFinish();
 }
}; // Raymarching
