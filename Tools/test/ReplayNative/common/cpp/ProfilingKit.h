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
// ProfilingRenderer.h
// Base Class for apps
//--------------------------------------------------------------------------------
#ifndef _PROFILINGKIT_H
#define _PROFILINGKIT_H

#include <jni.h>
#include <errno.h>

#include <vector>
#include <string>
#include <set>

#include <EGL/egl.h>
#include <GLES/gl.h>

#include <android/sensor.h>
#include <android/log.h>
#include <android_native_app_glue.h>
#include <android/native_window_jni.h>


#include "NDKHelper.h"
#include "ProfilingNDK.h"

#define LPGPU2LOG(...) __android_log_print(ANDROID_LOG_INFO,"LPGPU2: ",__VA_ARGS__)

#define BUFFER_OFFSET(i) ((char*)NULL + (i))



class TextureUnit;


class Texture {
 friend class TextureUnit;

//public: // HACK!
 GLuint texture = 0;

public:
 Texture() {}
~Texture() {clear();}

 void Load(const char* file) {
  clear();

  texture = LPGPU2::LoadTexture(file);
 }

 void unbind() {glBindTexture(GL_TEXTURE_2D,0);}

 void clear() {if (texture) glDeleteTextures(1,&texture);}
}; // Texture



class Program;



class TextureUnit {
 friend class Program;

 Program& program; GLuint unit; std::string name; GLuint id;

 void locate();

 TextureUnit(const TextureUnit&) = delete;

public:
 TextureUnit(Program& p, GLuint u, const char* s);

 void operator()(Texture& t) {
  glActiveTexture(GL_TEXTURE0 + unit);
  glBindTexture(GL_TEXTURE_2D,t.texture);
  glUniform1i(id,unit);
 }
}; // TextureUnit



class Program {
 friend class TextureUnit;

 Program(const Program&) = delete;

 GLuint program;
 GLuint vshader;
 GLuint fshader;

 void delete_shader (GLuint& shader);
 void delete_program();

 std::set<TextureUnit*> texture_units;

 void    add_texture_unit (TextureUnit& t) {texture_units.insert(&t);}
 void locate_texture_units()               {for(auto& t:texture_units) t->locate();}

public:
 Program():program(0),vshader(0),fshader(0) {}

~Program() {clear();}

 void clear();

 bool compile(const char* vpath, const char* fpath);

 void   bindAttribLocation(GLuint loc, const char* name) {       glBindAttribLocation(program, loc, name);}
 GLuint getUniformLocation(            const char* name) {return glGetUniformLocation(program,      name);}

 bool link();

 void use() {glUseProgram(program);}

 virtual void uniforms() {}
}; // Program



inline      TextureUnit::TextureUnit(Program& p, GLuint u, const char* s):program(p),unit(u),name(s),id(0) {p.add_texture_unit(*this);}

inline void TextureUnit::locate() {id = program.getUniformLocation(name.c_str());}



#if 0
class ProfilingRenderer {
protected:
 ndk_helper::TapCamera* camera_;

 ProfilingRenderer(const ProfilingRenderer&) = delete;

 ProfilingRenderer():camera_(nullptr) {}

 virtual ~ProfilingRenderer() {}

public:
 bool Bind(ndk_helper::TapCamera* camera);

 virtual void UpdateViewport() {}

 virtual void Init() {}
 virtual void Unload() {}

 virtual void Update(double time) {}
 virtual void Render(double time) {}
}; // ProfilingRenderer
#endif



class VertexAttribGuard {
 GLuint index;

public:
 VertexAttribGuard(GLuint ind, GLint siz, GLenum type, GLboolean norm, GLsizei stride, const GLvoid* pointer);

~VertexAttribGuard() {glDisableVertexAttribArray(index);}
}; // VertexAttribGuard



class BufferObject {
 BufferObject(const BufferObject&) = delete;

protected:
 GLuint bo_; GLsizei count; GLenum type;

 BufferObject(GLenum t):bo_(0),count(0),type(t) {}
~BufferObject() {if (bo_) {glDeleteBuffers(1,&bo_); bo_=0;}}

public:
 void Init(GLsizei num, GLsizei siz, const GLvoid* data, GLenum usage);

 void   bind() {glBindBuffer(type, bo_);}
 void unbind() {glBindBuffer(type, 0  );}

 GLsizei size() const {return count;}
}; // BufferObject



class Elements: public BufferObject {
protected:
 GLenum elem;

public:
 Elements(GLenum e):BufferObject(GL_ELEMENT_ARRAY_BUFFER),elem(e) {}

 void draw(GLenum mode, const GLvoid* indices) {glDrawElements(mode, count, elem, indices);}
}; // Elements



class Vertices: public BufferObject {
public:
 Vertices():BufferObject(GL_ARRAY_BUFFER) {}
}; // Vertices

#endif
