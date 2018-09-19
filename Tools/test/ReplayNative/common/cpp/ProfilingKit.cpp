/*
 * The MIT License
 *
 * Copyright (c) 2016-17 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */
/*
 * Copyright 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//--------------------------------------------------------------------------------
// ProfilingRenderer.cpp
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// Include files
//--------------------------------------------------------------------------------
#include "ProfilingRenderer.h"

#include <cmath>



void Program::delete_shader (GLuint& shader) {if (shader ) {glDeleteShader(shader ); shader =0;}}
void Program::delete_program()               {if (program) {glDeleteShader(program); program=0;}}



void Program::clear() {
 delete_shader(vshader);
 delete_shader(fshader);

 delete_program();
}



bool Program::compile(const char* vpath, const char* fpath) {
 clear();

 program = glCreateProgram();

 if (!ndk_helper::shader::CompileShader(&vshader, GL_VERTEX_SHADER  , vpath)) delete_program();
 if (!ndk_helper::shader::CompileShader(&fshader, GL_FRAGMENT_SHADER, fpath)) delete_program();

 if (program) {
  glAttachShader(program, vshader);
  glAttachShader(program, fshader);
 }

 return program;
}



bool Program::link() {
 bool r = ndk_helper::shader::LinkProgram(program);

 if (!r) {
  LOGI("Failed to link program: %d", program);

  delete_program();
 } else {
  uniforms();

  locate_texture_units();
 }

 delete_shader(vshader);
 delete_shader(fshader);

 return r;
}



VertexAttribGuard::VertexAttribGuard(GLuint ind, GLint siz, GLenum type, GLboolean norm, GLsizei stride, const GLvoid* pointer):index(ind) {
 glVertexAttribPointer(index,siz,type,norm,stride,pointer);

 glEnableVertexAttribArray(index);
}



void BufferObject::Init(GLsizei num, GLsizei siz, const GLvoid* data, GLenum usage) {
 count = num;

 glGenBuffers(1, &bo_);
 glBindBuffer(type, bo_);
 glBufferData(type, siz * num, data, usage);
 glBindBuffer(type, 0);
}



#if 0
bool ProfilingRenderer::Bind(ndk_helper::TapCamera* camera) {
 camera_ = camera;

 return true;
}
#endif
