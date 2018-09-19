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
// AppRenderer.cpp
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// Include files
//--------------------------------------------------------------------------------
#include "AppRenderer.h"
#include "ProfilingNDK.h"
#include "lpgpu2_api.h"
#include "ShaderAsm.cpp"

#include <cmath>
#include <string>
#include <fstream>
#include <cstdlib>
#include <thread>


// To turn of Shader psuedo assembly code ON / OFF, set USE_SHADER to 1 / 0 respectively
#define USE_SHADER_ASM 1


AppRenderer::AppRenderer():throttle(AppRenderer::FPS*AppRenderer::MULTI),start(false),finish(false) {
 std::this_thread::sleep_for(std::chrono::seconds(1));

 int err = glGetError();
}



AppRenderer::~AppRenderer() {
 Shutdown();
}



void AppRenderer::Shutdown() {
 ProfilingRenderer::Shutdown();
}



void ShaderMetaData(GLuint shader, GLenum type, int meta, size_t n, void* dat, int api = 0x100 /* APIID_GLES2 */) {
 lpgpu2_AssociateShaderMetaData(
  api,
  shader,
  type==GL_FRAGMENT_SHADER?
   LPGPU2_SHADER_TYPE_FRAGMENT:
   LPGPU2_SHADER_TYPE_VERTEX,
  meta,n,dat
 );
}



GLuint AppRenderer::LoadShader(GLenum type, std::string src) {
 GLuint shader = glCreateShader(type);

 const char* s = src.c_str();

 glShaderSource(shader, 1, &s, nullptr); //Not specifying 3rd parameter (size) could be troublesome..

 glCompileShader(shader);

#ifdef USE_SHADER_ASM
 ShaderAsm shaderAsm(shader,asmDict,"Asm/"+demo.getasmfile(src));

 ShaderMetaData(shader,type,LPGPU2_SHADER_META_TYPE_ASM   ,shaderAsm.source().size(),(void*)shaderAsm.source().data());//,0x100);
 ShaderMetaData(shader,type,LPGPU2_SHADER_META_TYPE_DEBUG ,shaderAsm.   debug_size(),       shaderAsm.debug ()       );//,0x200);
 ShaderMetaData(shader,type,LPGPU2_SHADER_META_TYPE_BINARY,15                       ,(void*)"< BINARY DATA >"        );//,0x400);
#endif

 return shader;
}



GLuint AppRenderer::LoadShaderFromFile(GLenum type, std::string file) {
 return LoadShader(type,strfromfile("Shaders/"+file));
}



int AppRenderer::GetScreenWidth () {int32_t viewport[4]; glGetIntegerv(GL_VIEWPORT, viewport); return viewport[2];}
int AppRenderer::GetScreenHeight() {int32_t viewport[4]; glGetIntegerv(GL_VIEWPORT, viewport); return viewport[3];}



#if 0
void AppRenderer::UpdateViewport() {
 int32_t viewport[4];

 glGetIntegerv(GL_VIEWPORT, viewport);
}



void AppRenderer::Startup() {
 glDisable(GL_CULL_FACE);

 glEnable(GL_DEPTH_TEST);
 glDepthFunc(GL_LEQUAL);

 glEnable(GL_BLEND);
 glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

// Settings
//  glFrontFace(GL_CCW);
}



void AppRenderer::Render(ProfilingContext& context) {
 double tim = context.getTime();
}
#endif
