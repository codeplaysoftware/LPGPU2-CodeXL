/*
 * The MIT License
 *
 * Copyright (c) 2016-17 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */
class Simple: public AppRenderer {
 GLuint vShader;
 GLuint fShader;
 GLuint sProgram;

public:
 void ReplayInit() {
#if 0
  const char* vSource =
   "attribute vec4 vPosition;\n"
   "\n"
   "void main() {\n"
   " gl_Position = vPosition;\n"
   "}\n";

  const char* fSource =
   "precision mediump float;\n"
   "\n"
   "void main() {\n"
   " gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
   "}\n";
#endif

  vShader = LoadShaderFromFile(GL_VERTEX_SHADER  , "triangle.vert");
  fShader = LoadShaderFromFile(GL_FRAGMENT_SHADER, "triangle.frag");

  sProgram = glCreateProgram();

  glAttachShader(sProgram, vShader);
  glAttachShader(sProgram, fShader);
  glBindAttribLocation(sProgram, 0, "vPosition");
  glBindAttribLocation(sProgram, 1, "vColor");
  glLinkProgram(sProgram);
  glClearColor(0.0, 0.0, 0.5, 1.0);
 }

 void ReplayRender(double tim) {
  float vVertices[] = {
    0.0,  0.5, 0.0,
   -0.5, -0.5, 0.0,
    0.5, -0.5, 0.0
  };

  glClear(GL_COLOR_BUFFER_BIT);
  glUseProgram(sProgram);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vVertices);
  glEnableVertexAttribArray(0);
  glDrawArrays(GL_TRIANGLES, 0, 3);
 }
};
