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
// Uber2.cpp
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// Include files
//--------------------------------------------------------------------------------
#include "Uber2.h"
#include "ProfilingNDK.h"

#include <cmath>



Uber2Earth::Uber2Earth()//double o, double r, double x, double y, double z)
:geometry_cloud(nullptr) {}//,offset(o),radius(r),x4D(x),y4D(y),z4D(z) {}



Uber2Earth::~Uber2Earth() {Shutdown();}

void Uber2Earth::Shutdown() {delete[] geometry_cloud; geometry_cloud = nullptr;}



#if 0
unsigned char* create_image(int w, int h) {
 unsigned char* dat = new unsigned char[4*w*h];

 for(int i=0,k=0;i<w;i++     )
 for(int j=0    ;j<h;j++,k+=4) {
  dat[k+0]=255;
  dat[k+1]=(int)(2.5*j);
  dat[k+2]=(int)(2.5*i);
  dat[k+3]=255;
 }

 return dat;
}
#endif



void Uber2::UpdateViewport(ProfilingContext& context) {
 int32_t viewport[4];

 glGetIntegerv(GL_VIEWPORT, viewport);

 const float CAM_NEAR =   0.5f; //     5.0f;
 const float CAM_FAR = 1000.0f; // 10000.0f;

 float aspect = static_cast<float>(viewport[2]) / static_cast<float>(viewport[3]);

 if (viewport[2] < viewport[3])
   context.mat_projection_ = ndk_helper::Mat4::Perspective(aspect,1.0f  , CAM_NEAR, CAM_FAR);
 else
   context.mat_projection_ = ndk_helper::Mat4::Perspective(1.0f,1/aspect, CAM_NEAR, CAM_FAR);
}



void Uber2::Update(ProfilingContext& context) {
 auto& mat_view_ = context.mat_view_;

 const float CAM_X =  0.0f;
 const float CAM_Y =  0.0f;
 const float CAM_Z = 70.0f; // 700.0f;

 double tim = context.getTime();

 double dist = 0; // 300*(1+std::sin(tim*0.2));

 mat_view_ = ndk_helper::Mat4::LookAt(ndk_helper::Vec3(CAM_X, CAM_Y, CAM_Z - dist),
                                      ndk_helper::Vec3(0.f, 0.f, 0.f),
                                      ndk_helper::Vec3(0.f, 1.f, 0.f));

 mat_view_ = mat_view_ * ndk_helper::Mat4::RotationY(0.1*tim);

 context.apply();
}



void Uber2Stars::Startup(ProfilingContext& context) {
 program_stars.compile("Shaders/stars.vsh", "Shaders/stars.fsh");

 program_stars.bindAttribLocation(UBER2_ATTRIB_Position,"Position");
 program_stars.bindAttribLocation(UBER2_ATTRIB_Normal  ,"Normal"  );
 program_stars.bindAttribLocation(UBER2_ATTRIB_Tex     ,"Tex"     );

 program_stars.link();

 const double radius = 800.0; // 9000.0;

 const int nx = 32, ny = 16;

 geometry_stars .Init(nx,ny,radius,GL_STATIC_DRAW,0.0);

 texture_stars.Load("Images/Nasa/Stars/starmap_8k.jpg");
}



void Uber2Earth::Startup(ProfilingContext& context) {
 glDisable(GL_CULL_FACE);

 glEnable(GL_DEPTH_TEST);
 glDepthFunc(GL_LEQUAL);

 glEnable(GL_BLEND);
 glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

// Settings
//  glFrontFace(GL_CCW);



 auto& mat_model_ = context.mat_model_;

 mat_model_ = ndk_helper::Mat4::Translation(0, 0, 0);

 ndk_helper::Mat4 mat = ndk_helper::Mat4::RotationX(0.8*M_PI)
                      * ndk_helper::Mat4::RotationY(    M_PI);

 mat_model_ = mat * mat_model_;



 program_earth.compile("Shaders/uber-earth.vsh", "Shaders/uber-earth.fsh");

 program_earth.bindAttribLocation(UBER2_ATTRIB_Position,"Position");
 program_earth.bindAttribLocation(UBER2_ATTRIB_Normal  ,"Normal"  );
 program_earth.bindAttribLocation(UBER2_ATTRIB_Tex     ,"Tex"     );

 program_earth.link();



 program_cloud.compile("Shaders/uber-cloud.vsh", "Shaders/uber-cloud.fsh");

 program_cloud.bindAttribLocation(UBER2_ATTRIB_Position,"Position");
 program_cloud.bindAttribLocation(UBER2_ATTRIB_Normal  ,"Normal"  );
 program_cloud.bindAttribLocation(UBER2_ATTRIB_Tex     ,"Tex"     );

 program_cloud.link();



 const double eps = 0.5;

 const int nx = 320, ny = 160;

 geometry_earth.Init(nx,ny,radius,GL_STATIC_DRAW,offset);




 num_cloud_layers = 5;

 geometry_cloud = new Uber2Geometry[num_cloud_layers];

 for(int i=0;i<num_cloud_layers;i++)
  geometry_cloud[i].Init(nx,ny,radius+(i+1)*eps,GL_STATIC_DRAW,offset);


// texture_night.Load("Images/Nasa/Night/dnb_land_ocean_ice.2012.3600x1800.jpg");
// texture_cloud.Load("Images/Nasa/Cloud/cloud_combined_2048.jpg");
// texture_earth.Load("Images/Nasa/Months/01-jan/world.topo.bathy.200401.3x5400x2700.png");
}



void Uber2Stars::Render(ProfilingContext& context) {
 auto& mat_projection_ = context.mat_projection_;
 auto& mat_view_       = context.mat_view_;

 ndk_helper::Mat4 mat_vp = mat_projection_ * mat_view_;


 {
  // Bind the buffers
  geometry_stars.bind();

  // Pass the vertex data
  int32_t stride = sizeof(UBER2_VERTEX);

  VertexAttribGuard guard_attrib_Position(UBER2_ATTRIB_Position, 4, GL_FLOAT, GL_FALSE, stride, BUFFER_OFFSET(0                  ));
  VertexAttribGuard guard_attrib_Normal  (UBER2_ATTRIB_Normal  , 3, GL_FLOAT, GL_FALSE, stride, BUFFER_OFFSET(4 * sizeof(GLfloat)));
  VertexAttribGuard guard_attrib_Tex     (UBER2_ATTRIB_Tex     , 2, GL_FLOAT, GL_FALSE, stride, BUFFER_OFFSET(7 * sizeof(GLfloat)));

  // Instruct pipeline to use our shader program
  program_stars.use();

  // Texturing...
  program_stars.surface(texture_stars);

  // Update uniforms
  glUniformMatrix4fv(program_stars.matrix_projection_, 1, GL_FALSE, mat_vp   .Ptr());
  glUniformMatrix4fv(program_stars.matrix_view_      , 1, GL_FALSE, mat_view_.Ptr());

  // Draw!
  geometry_stars.draw(GL_TRIANGLES, BUFFER_OFFSET(0));

  // Unbind
  geometry_stars.unbind();
 }
}



void Uber2Earth::Render(ProfilingContext& context) {//double tim) {
 // Set up Projection and Model-View matrices for the shaders
 double tim = context.getTime();

 auto& mat_projection_ = context.mat_projection_;
 auto& mat_view_       = context.mat_view_;

 ndk_helper::Mat4 mat_vp = mat_projection_ * mat_view_;


 {
  // Bind the geometry buffers
  geometry_earth.bind();

  // Pass the vertex data
  int32_t stride = sizeof(UBER2_VERTEX);

  VertexAttribGuard guard_attrib_Position(UBER2_ATTRIB_Position, 4, GL_FLOAT, GL_FALSE, stride, BUFFER_OFFSET(0                  ));
  VertexAttribGuard guard_attrib_Normal  (UBER2_ATTRIB_Normal  , 3, GL_FLOAT, GL_FALSE, stride, BUFFER_OFFSET(4 * sizeof(GLfloat)));
  VertexAttribGuard guard_attrib_Tex     (UBER2_ATTRIB_Tex     , 2, GL_FLOAT, GL_FALSE, stride, BUFFER_OFFSET(7 * sizeof(GLfloat)));

  // Instruct pipeline to use our shader program
  program_earth.use();

  // Texturing...
  program_earth.surface0(*texture_earth);
  program_earth.surface1(*texture_night);

  // Update uniforms
  glUniformMatrix4fv(program_earth.matrix_projection_, 1, GL_FALSE, mat_vp   .Ptr());
  glUniformMatrix4fv(program_earth.matrix_view_      , 1, GL_FALSE, mat_view_.Ptr());

  glUniform1f       (program_earth.interpolation_    , tim);
  glUniform1f       (program_earth.hcphase_          , tim+offset);

  // Draw!
  geometry_earth.draw(GL_TRIANGLES, BUFFER_OFFSET(0));

  // Unbind
  geometry_earth.unbind();
 }


 for(int i=0;i<num_cloud_layers;i++) {
  geometry_cloud[i].bind();

  int32_t stride = sizeof(UBER2_VERTEX);

  VertexAttribGuard guard_attrib_Position(UBER2_ATTRIB_Position, 4, GL_FLOAT, GL_FALSE, stride, BUFFER_OFFSET(0                  ));
  VertexAttribGuard guard_attrib_Normal  (UBER2_ATTRIB_Normal  , 3, GL_FLOAT, GL_FALSE, stride, BUFFER_OFFSET(4 * sizeof(GLfloat)));
  VertexAttribGuard guard_attrib_Tex     (UBER2_ATTRIB_Tex     , 2, GL_FLOAT, GL_FALSE, stride, BUFFER_OFFSET(7 * sizeof(GLfloat)));

  // Instruct pipeline to use our shader program
  program_cloud.use();

  // Texturing...
  program_cloud.surface(*texture_cloud);

  // Update uniforms
  glUniformMatrix4fv(program_cloud.matrix_projection_, 1, GL_FALSE, mat_vp   .Ptr());
  glUniformMatrix4fv(program_cloud.matrix_view_      , 1, GL_FALSE, mat_view_.Ptr());

  double u = i*1.0/num_cloud_layers; const double speed = 0.02;

  glUniform1f       (program_cloud.phase_            , tim*speed*(u-0.5)+u);
  glUniform1f       (program_cloud.A_                ,                   u);
  glUniform1f       (program_cloud.B_                ,                 1-u);

  glUniform1f       (program_cloud.hcphase_          , tim+offset);

  // Draw!
  geometry_cloud[i].draw(GL_TRIANGLES, BUFFER_OFFSET(0));

  // Unbind
  geometry_cloud[i].unbind();
 }
}



Uber2Program::Uber2Program() {}

void Uber2Program::uniforms() {
 matrix_projection_ = getUniformLocation("uPMatrix");
 matrix_view_       = getUniformLocation("uMVMatrix");
}



Uber2ProgramStars::Uber2ProgramStars():
 surface(*this,0,"surface") // texture unit 0
{}

void Uber2ProgramStars::uniforms() {
 Uber2Program::uniforms();
}



Uber2ProgramEarth::Uber2ProgramEarth():
 surface0(*this,0,"surface0"), // texture unit  0
 surface1(*this,1,"surface1")  // texture unit  1
{}



void Uber2ProgramEarth::uniforms() {
 Uber2Program::uniforms();

 interpolation_ = getUniformLocation("interpolation");
 hcphase_       = getUniformLocation("hcphase");
}



Uber2ProgramCloud::Uber2ProgramCloud():
 surface(*this,0,"surface") // texture unit 0
{}

void Uber2ProgramCloud::uniforms() {
 Uber2Program::uniforms();

 phase_   = getUniformLocation("phase");
 A_       = getUniformLocation("A");
 B_       = getUniformLocation("B");

 hcphase_ = getUniformLocation("hcphase");
}



void Uber2Elements::Init(std::size_t nx, std::size_t ny, GLenum mode) {
  xsiz = nx;
  ysiz = ny;

//std::size_t num = 3*(nx-1)*(ny-1);
  std::size_t num = 6*(nx-1)*(ny-1);

//int16_t* dat = new int16_t[num];
  int32_t* dat = new int32_t[num];

  for(int i=1,o=0;i<nx;i++    )
  for(int j=1    ;j<ny;j++,o++) {
// dat[3*o  ]=index(i  ,j  ,nx,ny);
// dat[3*o+1]=index(i-1,j  ,nx,ny);
// dat[3*o+2]=index(i  ,j-1,nx,ny);

   dat[6*o  ]=index(i  ,j  ,nx,ny);
   dat[6*o+1]=index(i-1,j  ,nx,ny);
   dat[6*o+2]=index(i  ,j-1,nx,ny);
   dat[6*o+3]=index(i  ,j-1,nx,ny);
   dat[6*o+4]=index(i-1,j  ,nx,ny);
   dat[6*o+5]=index(i-1,j-1,nx,ny);
  }

//Elements::Init(num,sizeof(int16_t),dat,mode);
  Elements::Init(num,sizeof(int32_t),dat,mode);

  delete[] dat;
}



void Uber2Vertices::Init(Uber2Elements& elem, double radius, GLenum mode, double offset) {
  xsiz = elem.xsize();
  ysiz = elem.ysize();

  UBER2_VERTEX* dat = new UBER2_VERTEX[xsiz*ysiz];

  for(int i=0,o=0;i<xsiz;i++    ) {float u = i/(xsiz-1.0);
  for(int j=0    ;j<ysiz;j++,o++) {float v = j/(ysiz-1.0);
   coords(u,v,dat[o].nrm);

   dat[o].pos[0] = radius * dat[o].nrm[0];
   dat[o].pos[1] = radius * dat[o].nrm[1];
   dat[o].pos[2] = radius * dat[o].nrm[2];
   dat[o].pos[3] = offset * (dat[o].nrm[0]+2)/3;

// radius * (dat[o].nrm[2]+2)/3; // could try almost anything here as long is it varies smoothly with position

   dat[o].tex[0] =   u;
   dat[o].tex[1] = 1-v;
  }}

  Vertices::Init(xsiz*ysiz,sizeof(UBER2_VERTEX),dat,mode);

  delete[] dat;
}



void Uber2Geometry::Init(int nx, int ny, double radius, GLenum mode, double offset) {
 elements.Init(nx,ny          ,mode);
 vertices.Init(elements,radius,mode,offset);
}

void Uber2Geometry::bind() {
 elements.bind();
 vertices.bind();
}

void Uber2Geometry::unbind() {
 elements.unbind();
 vertices.unbind();
}
