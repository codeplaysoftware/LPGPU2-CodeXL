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
// Globe.h
// 
// Globe class implementing a specific ProfilingRenderer for this application
//---------------------------------------------------------------------------------
#ifndef _GLOBERENDERER_H
#define _GLOBERENDERER_H



#include "ProfilingRenderer.h"

#include <cmath>



struct GLOBE_VERTEX {
 float pos[3];
 float nrm[3];
 float tex[2];
};



enum GLOBE_SHADER_ATTRIBUTES {
 GLOBE_ATTRIB_Position,
 GLOBE_ATTRIB_Normal,
 GLOBE_ATTRIB_Tex,
};



class GlobeProgram: public Program {
public:
 GLuint matrix_projection_;
 GLuint matrix_view_;

protected:
 GlobeProgram();

 void uniforms();
}; // GlobeProgram



class GlobeProgramStars: public GlobeProgram {
public:
 TextureUnit surface;

 GlobeProgramStars();

 void uniforms();
}; // GlobeProgramStars



class GlobeProgramEarth: public GlobeProgram {
public:
 TextureUnit surface0;
 TextureUnit surface1;

 GLuint interpolation_;

 GlobeProgramEarth();

 void uniforms();
}; // GlobeProgramEarth



class GlobeProgramCloud: public GlobeProgram {
public:
 TextureUnit surface;

 GLuint phase_;
 GLuint A_; // coefficients of Ax2+bx+c for offsetting clouds
 GLuint B_;

 GlobeProgramCloud();

 void uniforms();
}; // GlobeProgramCloud



class GlobeElements: public Elements {
 int index(int i, int n) {while(i<0) i+=n; i%=n; return i;}

 int index(int i, int j, int nx, int ny) {return index(i,nx)*ny+index(j,ny);}

 std::size_t xsiz,ysiz;

public:
 GlobeElements():Elements(GL_UNSIGNED_INT) {}

 void Init(std::size_t nx, std::size_t ny, GLenum mode);

 std::size_t xsize() const {return xsiz;}
 std::size_t ysize() const {return ysiz;}
}; // GlobeElements



class GlobeVertices: public Vertices {
 std::size_t xsiz,ysiz;

 void coords(float u, float v, float* p) {
  float lon = M_PI * (u - 0.5) * 2; // [-180 .. 180] (but in radians)
  float lat = M_PI * (v - 0.5);     // [ -90 ..  90]

  p[0] = std::sin(lon) * std::cos(lat); // spherical polar coords
  p[1] =                 std::sin(lat);
  p[2] = std::cos(lon) * std::cos(lat);
 }


public:
 GlobeVertices() {}

 void Init(GlobeElements& elem, double radius, GLenum mode);
}; // GlobeVertices



class GlobeGeometry {
 GlobeElements elements;
 GlobeVertices vertices;

public:
 GlobeGeometry() {}

 void Init(int nx, int ny, double radius, GLenum mode);

 void draw(GLenum mode, const GLvoid* indices) {elements.draw(mode,indices);}

 void   bind();
 void unbind();
}; // GlobeGeometry



class Globe: public ProfilingRenderer {
#if 0
 ndk_helper::Mat4 mat_projection_;
 ndk_helper::Mat4 mat_view_;
 ndk_helper::Mat4 mat_model_;
#endif

 bool use_camera_transform_ = true;

 GlobeProgramStars program_stars;
 GlobeProgramEarth program_earth;
 GlobeProgramCloud program_cloud;

 GlobeGeometry    geometry_stars;
 GlobeGeometry    geometry_earth;
 GlobeGeometry*   geometry_cloud;

 int num_cloud_layers;

 Texture texture_stars;
 Texture texture_earth;
 Texture texture_cloud;

 Texture texture_night;

 Texture texture_month[12];

 void CameraTransformOn () {use_camera_transform_ = true ;}
 void CameraTransformOff() {use_camera_transform_ = false;}

public:
 Globe();
~Globe();

 void Startup       (ProfilingContext&) override;
 void Shutdown      ()                  override;

 void UpdateViewport(ProfilingContext&) override;
 void Update        (ProfilingContext&) override;
 void Render        (ProfilingContext&) override;
}; // Globe

#endif
