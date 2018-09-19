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
// Uber.h
// 
// Uber class implementing a specific ProfilingRenderer for this application
//---------------------------------------------------------------------------------
#ifndef _UBERRENDERER_H
#define _UBERRENDERER_H



#include "ProfilingRenderer.h"

#include <cmath>



struct UBER_VERTEX {
 float pos[4];
 float nrm[3];
 float tex[2];
};



enum UBER_SHADER_ATTRIBUTES {
 UBER_ATTRIB_Position,
 UBER_ATTRIB_Normal,
 UBER_ATTRIB_Tex,
};



class UberProgram: public Program {
public:
 GLuint matrix_projection_;
 GLuint matrix_view_;

protected:
 UberProgram();

 void uniforms();
}; // UberProgram



class UberProgramStars: public UberProgram {
public:
 TextureUnit surface;

 UberProgramStars();

 void uniforms();
}; // UberProgramStars



class UberProgramEarth: public UberProgram {
public:
 TextureUnit surface0;
 TextureUnit surface1;

 GLuint interpolation_;
 GLuint hcphase_; // hypercube phase

 UberProgramEarth();

 void uniforms();
}; // UberProgramEarth



class UberProgramCloud: public UberProgram {
public:
 TextureUnit surface;

 GLuint phase_;
 GLuint A_; // coefficients of Ax2+bx+c for offsetting clouds
 GLuint B_;

 GLuint hcphase_; // hypercube phase

 UberProgramCloud();

 void uniforms();
}; // UberProgramCloud



class UberElements: public Elements {
 int index(int i, int n) {while(i<0) i+=n; i%=n; return i;}

 int index(int i, int j, int nx, int ny) {return index(i,nx)*ny+index(j,ny);}

 std::size_t xsiz,ysiz;

public:
 UberElements():Elements(GL_UNSIGNED_INT) {}

 void Init(std::size_t nx, std::size_t ny, GLenum mode);

 std::size_t xsize() const {return xsiz;}
 std::size_t ysize() const {return ysiz;}
}; // UberElements



class UberVertices: public Vertices {
 std::size_t xsiz,ysiz;

 void coords(float u, float v, float* p) {
  float lon = M_PI * (u - 0.5) * 2; // [-180 .. 180] (but in radians)
  float lat = M_PI * (v - 0.5);     // [ -90 ..  90]

  p[0] = std::sin(lon) * std::cos(lat); // spherical polar coords
  p[1] =                 std::sin(lat);
  p[2] = std::cos(lon) * std::cos(lat);
 }


public:
 UberVertices() {}

 void Init(UberElements& elem, double radius, GLenum mode, double offset);
}; // UberVertices



class UberGeometry {
 UberElements elements;
 UberVertices vertices;

public:
 UberGeometry() {}

 void Init(int nx, int ny, double radius, GLenum mode, double offset);

 void draw(GLenum mode, const GLvoid* indices) {elements.draw(mode,indices);}

 void   bind();
 void unbind();
}; // UberGeometry



class UberScene {
public:
 virtual void Startup (ProfilingContext&)=0;
 virtual void Shutdown()                 =0;

 virtual void Update  (ProfilingContext&)=0;
 virtual void Render  (ProfilingContext&)=0;
}; // UberScene



class UberStars: public UberScene {
 UberProgramStars program_stars;
 UberGeometry    geometry_stars;
 Texture          texture_stars;

public:
 void Startup (ProfilingContext&);
 void Shutdown() {}

 void Update  (ProfilingContext&) {}
 void Render  (ProfilingContext&);
}; // UberStars



class UberEarth: public UberScene {
 UberProgramEarth program_earth;
 UberProgramCloud program_cloud;

 UberGeometry    geometry_earth;
 UberGeometry*   geometry_cloud;

 Texture          texture_earth;
 Texture          texture_cloud;
 Texture          texture_night;

 int num_cloud_layers;

 double offset,radius,x4D,y4D,z4D;

public:
 UberEarth();//double o, double r, double p);
~UberEarth();

 void Init(double o, double r, double x, double y, double z) {
  offset = o;
  radius = r;
  x4D    = x;
  y4D    = y;
  z4D    = z;
 }

 void Startup (ProfilingContext&) override;
 void Shutdown()                  override;

 void Update  (ProfilingContext& c) override {c.mat_model_ = ndk_helper::Mat4::Translation(x4D,y4D,z4D);}//offset,0.0f,0.0f);}
 void Render  (ProfilingContext&) override;
}; // UberEarth



class Uber: public ProfilingRenderer {
 const double radius = 30;

 enum {N=2};

 UberStars stars;
 UberEarth earth[N][N][N];

 void Reset(ProfilingContext& c) {
  c.mat_model_ = ndk_helper::Mat4();
 }

public:
 Uber() {
  for(int i=0;i<N;i++)
  for(int j=0;j<N;j++)
  for(int k=0;k<N;k++) {
   double u = N*(i+N*j+N*N*k)/(N*N*N-1.0)-1.0;

   earth[i][j][k].Init(radius*u,radius,
    radius*(N*i-(N-1)),
    radius*(N*j-(N-1)),
    radius*(N*k-(N-1))
   );
  }
 }

 void UpdateViewport(ProfilingContext&  ) override;

 void Startup       (ProfilingContext& c) override {
  stars.Startup (c);

  for(int i=0;i<N;i++)
  for(int j=0;j<N;j++)
  for(int k=0;k<N;k++)
   earth[i][j][k] .Startup (c);

  c.CameraTransformOff();
 }

 void Shutdown() override {
  stars .Shutdown( );

  for(int i=0;i<N;i++)
  for(int j=0;j<N;j++)
  for(int k=0;k<N;k++)
   earth[i][j][k] .Shutdown( );

  ProfilingRenderer::Shutdown();
 }

 void Update        (ProfilingContext&  ) override;

 void Render        (ProfilingContext& c) override {
  stars.Update(c); Update(c); stars .Render(c);

  for(int i=0;i<N;i++)
  for(int j=0;j<N;j++)
  for(int k=0;k<N;k++) {
   earth[i][j][k].Update(c); Update(c); earth[i][j][k].Render(c);
  }
 }
}; // Uber



#endif
