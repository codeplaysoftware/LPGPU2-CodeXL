/*
 * The MIT License
 *
 * Copyright (c) 2016-17 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */
class Triangle1: public AppRenderer {

 const GLchar* vsrc =
#if 0
  "#version 330 core\n"
  "layout (location = 0) in vec3 position;\n"
  "\n"
  "void main() {\n"
  " gl_Position = vec4(position,1.0f);\n"
  "}\n";
#endif

  "attribute vec4 vPosition;\n"
  "\n"
  "void main() {\n"
  " gl_Position = vPosition;\n"
  "}\n";

 const GLchar* fsrc =
#if 0
  "#version 330 core\n"
  "\n"
  "out vec4 color;\n"
  "\n"
  "void main() {\n"
  " color = vec4(1.0f,0.5f,0.2f,1.0f);\n"
  "}\n";
#endif

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

public:
 Triangle1() {}

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
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

  glUseProgram(program);
  glBindVertexArray(VAO);
  glDrawArrays(GL_TRIANGLES,0,3);
  glBindVertexArray(0);
 }
}; // Triangle1
