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
// Uber2.h
// 
// Uber2 class implementing a specific ProfilingRenderer for this application
//---------------------------------------------------------------------------------
#ifndef _UBER2RENDERER_H
#define _UBER2RENDERER_H



#include "ProfilingRenderer.h"

#include <cmath>



struct UBER2_VERTEX {
 float pos[4];
 float nrm[3];
 float tex[2];
};



enum UBER2_SHADER_ATTRIBUTES {
 UBER2_ATTRIB_Position,
 UBER2_ATTRIB_Normal,
 UBER2_ATTRIB_Tex,
};



class Uber2Program: public Program {
public:
 GLuint matrix_projection_;
 GLuint matrix_view_;

protected:
 Uber2Program();

 void uniforms();
}; // Uber2Program



class Uber2ProgramStars: public Uber2Program {
public:
 TextureUnit surface;

 Uber2ProgramStars();

 void uniforms();
}; // Uber2ProgramStars



class Uber2ProgramEarth: public Uber2Program {
public:
 TextureUnit surface0;
 TextureUnit surface1;

 GLuint interpolation_;
 GLuint hcphase_; // hypercube phase

 Uber2ProgramEarth();

 void uniforms();
}; // Uber2ProgramEarth



class Uber2ProgramCloud: public Uber2Program {
public:
 TextureUnit surface;

 GLuint phase_;
 GLuint A_; // coefficients of Ax2+bx+c for offsetting clouds
 GLuint B_;

 GLuint hcphase_; // hypercube phase

 Uber2ProgramCloud();

 void uniforms();
}; // Uber2ProgramCloud



class Uber2Elements: public Elements {
 int index(int i, int n) {while(i<0) i+=n; i%=n; return i;}

 int index(int i, int j, int nx, int ny) {return index(i,nx)*ny+index(j,ny);}

 std::size_t xsiz,ysiz;

public:
 Uber2Elements():Elements(GL_UNSIGNED_INT) {}

 void Init(std::size_t nx, std::size_t ny, GLenum mode);

 std::size_t xsize() const {return xsiz;}
 std::size_t ysize() const {return ysiz;}
}; // Uber2Elements



class Uber2Vertices: public Vertices {
 std::size_t xsiz,ysiz;

 void coords(float u, float v, float* p) {
  float lon = M_PI * (u - 0.5) * 2; // [-180 .. 180] (but in radians)
  float lat = M_PI * (v - 0.5);     // [ -90 ..  90]

  p[0] = std::sin(lon) * std::cos(lat); // spherical polar coords
  p[1] =                 std::sin(lat);
  p[2] = std::cos(lon) * std::cos(lat);
 }


public:
 Uber2Vertices() {}

 void Init(Uber2Elements& elem, double radius, GLenum mode, double offset);
}; // Uber2Vertices



class Uber2Geometry {
 Uber2Elements elements;
 Uber2Vertices vertices;

public:
 Uber2Geometry() {}

 void Init(int nx, int ny, double radius, GLenum mode, double offset);

 void draw(GLenum mode, const GLvoid* indices) {elements.draw(mode,indices);}

 void   bind();
 void unbind();
}; // Uber2Geometry



class Uber2Scene {
public:
 virtual void Startup (ProfilingContext&)=0;
 virtual void Shutdown()                 =0;

 virtual void Update  (ProfilingContext&)=0;
 virtual void Render  (ProfilingContext&)=0;
}; // Uber2Scene



class Uber2Stars: public Uber2Scene {
 Uber2ProgramStars program_stars;
 Uber2Geometry    geometry_stars;
 Texture          texture_stars;

public:
 void Startup (ProfilingContext&);
 void Shutdown() {}

 void Update  (ProfilingContext&) {}
 void Render  (ProfilingContext&);
}; // Uber2Stars



class Uber2Earth: public Uber2Scene {
 Uber2ProgramEarth program_earth;
 Uber2ProgramCloud program_cloud;

 Uber2Geometry    geometry_earth;
 Uber2Geometry*   geometry_cloud;

 Texture*          texture_earth = nullptr;
 Texture*          texture_cloud = nullptr;
 Texture*          texture_night = nullptr;

 int num_cloud_layers;

 double offset,radius,x4D,y4D,z4D;

public:
 Uber2Earth();//double o, double r, double p);
~Uber2Earth();

 void Init(double o, double r, double x, double y, double z, Texture* earth, Texture* cloud, Texture* night) {
  offset = o;
  radius = r;
  x4D    = x;
  y4D    = y;
  z4D    = z;

  texture_earth = earth;
  texture_cloud = cloud;
  texture_night = night;
 }

 void Startup (ProfilingContext&) override;
 void Shutdown()                  override;

 void Update  (ProfilingContext& c) override {c.mat_model_ = ndk_helper::Mat4::Translation(x4D,y4D,z4D);}//offset,0.0f,0.0f);}
 void Render  (ProfilingContext&) override;
}; // Uber2Earth



class Uber2: public ProfilingRenderer {
 const double radius = 30;

 enum {N=2};

 Uber2Stars stars;
 Uber2Earth earth[N][N][N];

 void Reset(ProfilingContext& c) {
  c.mat_model_ = ndk_helper::Mat4();
 }

 Texture texture_earth;
 Texture texture_cloud;
 Texture texture_night;

public:
 Uber2() {
 }

 void UpdateViewport(ProfilingContext&  ) override;

 void Startup       (ProfilingContext& c) override {
  texture_earth.Load("Images/Nasa/Months/01-jan/world.topo.bathy.200401.3x5400x2700.png");
  texture_cloud.Load("Images/Nasa/Cloud/cloud_combined_2048.jpg"); 
  texture_night.Load("Images/Nasa/Night/dnb_land_ocean_ice.2012.3600x1800.jpg");

  for(int i=0;i<N;i++)
  for(int j=0;j<N;j++)
  for(int k=0;k<N;k++) {
   double u = N*(i+N*j+N*N*k)/(N*N*N-1.0)-1.0;

   earth[i][j][k].Init(radius*u,radius,
    radius*(N*i-(N-1)),
    radius*(N*j-(N-1)),
    radius*(N*k-(N-1)),

    &texture_earth,
    &texture_cloud,
    &texture_night
   );
  }

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
}; // Uber2



#endif
