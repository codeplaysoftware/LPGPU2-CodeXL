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
#ifndef _PROFILINGRENDERER_H
#define _PROFILINGRENDERER_H

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



#include "ProfilingKit.h"



class ProfilingContext {
 ndk_helper::TapCamera* camera_;

 double tim;

public:
 ProfilingContext():camera_(nullptr) {}

 bool Bind(ndk_helper::TapCamera* camera) {camera_ = camera; return true;}

 void set_time(double t) {tim=t;}

 ndk_helper::TapCamera* getCamera() {return camera_;}

 double getTime() {return tim;}
}; // ProfilingContext



class ProfilingRenderer {
 ProfilingRenderer(const ProfilingRenderer&) = delete;

protected:
 ProfilingRenderer() {}

 virtual ~ProfilingRenderer() {}

public:
 virtual void UpdateViewport() {}

 virtual void Startup () {}
 virtual void Shutdown() {}

 virtual void Update(ProfilingContext&) {}
 virtual void Render(ProfilingContext&) {}
}; // ProfilingRenderer

#endif
