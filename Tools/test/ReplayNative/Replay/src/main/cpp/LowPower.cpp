/*
 * The MIT License
 *
 * Copyright (c) 2016-17 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */



#include "Thrasher.hpp"



void LogPrint(const char *format, ...)
{
  va_list args;
  va_start(args, format);
  __android_log_vprint(ANDROID_LOG_INFO, "LPGPU2 LowPower", format, args);
  va_end(args);
}




class LowPower: public AppRenderer {

 class Thrasher: public ::Thrasher {
  void task() override {double x=0; for(size_t i=1<<20;i;i--) x=std::cos(x);}

 public:
  Thrasher(size_t n, uint mask): ::Thrasher(n,mask) {} // number of threads in 'thrasher' threadpool
 }; // Thrasher


 const GLchar* vsrc =
  "attribute vec4 vPosition;\n"
  "\n"
  "void main() {\n"
  " gl_Position = vPosition;\n"
  "}\n";

 const GLchar* fsrc =
  "precision mediump float;\n"
  "\n"
  "void main() {\n"
  "\n"
  " gl_FragColor = vec4(0.0,0.5,0.0,1.0);\n"
  "}\n";
  
 GLfloat vtx[9] = {
  -1.0f,-1.0f, 0.0f,
   3.0f,-1.0f, 0.0f,
  -1.0f, 3.0f, 0.0f
 };

 GLuint program;

 GLuint VBO,VAO;

 Thrasher thrasher;

 enum {NTHREADS = 0}; // 0 means thrashing is turned off

public:
 LowPower():thrasher(NTHREADS,MASK) {}

 void ReplayInit() {
  GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vShader,1,&vsrc,NULL);
  glCompileShader(vShader);

  GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);

  glShaderSource(fShader,1,&fsrc ,NULL);

  glCompileShader(fShader);


  GLint isCompiled = 0;
  glGetShaderiv(fShader, GL_COMPILE_STATUS, &isCompiled);
  if(isCompiled == GL_FALSE)
  {
	GLint maxLength = 0;
	glGetShaderiv(fShader, GL_INFO_LOG_LENGTH, &maxLength);

	// The maxLength includes the NULL character
	std::vector<GLchar> errorLog(maxLength);
	glGetShaderInfoLog(fShader, maxLength, &maxLength, &errorLog[0]);

        LogPrint("Shader error: %s",&errorLog[0]);
  }

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
  thrasher.work();

  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

  glUseProgram(program);
  glBindVertexArray(VAO);
  glDrawArrays(GL_TRIANGLES,0,3);
  glBindVertexArray(0);

  lpgpu2_LogUserCounter(LPGPU2_USER_COUNTER_0,(std::uint32_t)throttle.fps());

  thrasher.wait();
 }
}; // LowPower
