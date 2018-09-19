/*
 * The MIT License
 *
 * Copyright (c) 2016-17 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */
class Correctness: public AppRenderer {

 const GLchar* vsrc =
  "attribute vec4 vPosition;\n"
  "\n"
  "void main() {\n"
  " gl_Position = vPosition;\n"
  "}\n";

 const GLchar* fsrc =
  "void main() {\n"
  " gl_FragColor = vec4(1.0,0.0,0.0,1.0);\n"
  "}\n";
  

 GLfloat vtx[9] = {
  -0.5f,-0.5f, 0.0f,
   0.5f,-0.5f, 0.0f,
   0.0f, 0.5f, 0.0f
 };

 GLuint program;

 GLuint VBO,VAO;

 int countdown; // number of frames executed - default is 1 - ideal for debugging state
                // Note that the application will not terminate, you will have to stop collecting
                // after the animation stops. It's quite difficult to get a NativeActivity to exit.

                // Note also that with a single frame you will not see anything because the app
                // is double buffered, but that is not a problem because this app is for state tracking

public:
 Correctness(int n = 1):countdown(n) {}

 void ReplayInit() {
  GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vShader,1,&vsrc,NULL);
  glCompileShader(vShader);

  GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fShader,1,&fsrc,NULL);
  glCompileShader(fShader);

  program = glCreateProgram();
  glAttachShader(program,vShader);
  glAttachShader(program,fShader);
  glLinkProgram(program);

  glDeleteShader(vShader);
  glDeleteShader(fShader);

  glGenVertexArrays(1,&VAO);
  glGenBuffers(1,&VBO);

  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER,VBO);
  glBufferData(GL_ARRAY_BUFFER,sizeof(vtx),vtx,GL_STATIC_DRAW);

  glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(GLfloat),(GLvoid*)0);
  glEnableVertexAttribArray(0);

  glBindVertexArray(0); // unbind

  glClearColor(0.125,0.125,1,1);
 }

 void ReplayRender(double tim) {
  if (!QuitRequested()) {
   glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

   glUseProgram(program);
   glBindVertexArray(VAO);
   glDrawArrays(GL_TRIANGLES,0,3);
   glBindVertexArray(0);
  }

  if (!--countdown) {
   RequestQuit();

   glDeleteProgram(program);
   glDeleteBuffers(1,&VBO);
  }
 }
}; // Correctness
