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

//---------------------------------------------------------------------------------
// AppRenderer.h
// 
// AppRenderer class implementing a specific ProfilingRenderer for this application
//---------------------------------------------------------------------------------
#ifndef _APPRENDERER_H
#define _APPRENDERER_H



#include "ProfilingRenderer.h"
#include "ShaderAsm.hpp"
#include "Throttle.hpp"
#include "lpgpu2_api.h"

#include <cmath>



class AppRenderer: public ProfilingRenderer {
public:
 // FPS   = frames per second cap (hardware is generally limited to 60fps, so a default of 120 is effectively deristricted)
 // MULTI = how many times scene is renderered per frame (profiling functionality to circumvent choreographer vertical sync limit)
 // MASK  = CPU mask. If it is 0, thread affinity of the main thread will not be set
 enum {FPS = 120, MULTI = 1, MASK = 0};

protected:
 Throttle throttle;

 GLuint LoadShader        (GLenum type, std::string src );
 GLuint LoadShaderFromFile(GLenum type, std::string file);

 int GetScreenWidth ();
 int GetScreenHeight();

 AsmDictOne    asmDict; // also AsmDictTwo
 ShaderAsmSetDemo demo;

 bool start,finish;

public:
 AppRenderer();
~AppRenderer();

 void Startup       ()            override {
  if (!start) {ReplayInit(); start=true;}

  if (MASK) lpgpu2_SetThreadAffinityMask(MASK);
 }

 void Shutdown      ()            override;

 void UpdateViewport()            override {if (!start) {ReplayInit(); start=true;}}

 void Render(ProfilingContext& c) override {
  for(int i=MULTI;i;i--) {throttle.wait(); ReplayRender(c.getTime());}

  lpgpu2_LogAffinityCounter();
 }

 bool QuitRequested() {return finish;}

 void RequestQuit() {finish=true;}

 virtual void ReplayInit  ()        =0;
 virtual void ReplayRender(double t)=0;
}; // AppRenderer

#endif
