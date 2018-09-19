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
// Globe.cpp
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// Include files
//--------------------------------------------------------------------------------
#include "Globe.h"
#include "ProfilingNDK.h"

#include <cmath>



//--------------------------------------------------------------------------------
// Ctor
//--------------------------------------------------------------------------------
Globe::Globe():geometry_cloud(nullptr) {
// CameraTransformOff();
   CameraTransformOn ();
}



//--------------------------------------------------------------------------------
// Dtor
//--------------------------------------------------------------------------------
Globe::~Globe() {
 Shutdown();
}



void Globe::Shutdown() {
 delete[] geometry_cloud; geometry_cloud = nullptr;

 ProfilingRenderer::Shutdown();
}



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



void Globe::UpdateViewport(ProfilingContext& context) {
 int32_t viewport[4];

 glGetIntegerv(GL_VIEWPORT, viewport);

 const float CAM_NEAR = 5.f;
 const float CAM_FAR = 10000.f;

 float aspect = static_cast<float>(viewport[2]) / static_cast<float>(viewport[3]);

 auto& mat_projection_ = context.mat_projection_;

 if (viewport[2] < viewport[3])
   mat_projection_ = ndk_helper::Mat4::Perspective(aspect, 1.0f, CAM_NEAR, CAM_FAR);
 else
   mat_projection_ = ndk_helper::Mat4::Perspective(1.0f, aspect, CAM_NEAR, CAM_FAR);
}



void Globe::Update(ProfilingContext& context) {//double tim) {
 const float CAM_X = 0.f;
 const float CAM_Y = 0.f;
 const float CAM_Z = 700.f;

 double tim = context.getTime();

 double dist = 300; // 300*(1+std::sin(tim*0.2));

 auto& mat_view_  = context.mat_view_;
 auto& mat_model_ = context.mat_model_;

 mat_view_ = ndk_helper::Mat4::LookAt(ndk_helper::Vec3(CAM_X, CAM_Y, CAM_Z - dist),
                                      ndk_helper::Vec3(0.f, 0.f, 0.f),
                                      ndk_helper::Vec3(0.f, 1.f, 0.f));

 auto camera_ = context.getCamera();


 if (camera_) {
   camera_->Update(tim);

   if (use_camera_transform_) mat_view_ = camera_->GetTransformMatrix() * mat_view_;

   mat_view_ = mat_view_ * camera_->GetRotationMatrix() * mat_model_;
 } else {
   mat_view_ = mat_view_ * mat_model_;
 }

 mat_view_ = mat_view_ * ndk_helper::Mat4::RotationY(0.02*tim);
}



void Globe::Startup(ProfilingContext& context) {
 glDisable(GL_CULL_FACE);

 glEnable(GL_DEPTH_TEST);
 glDepthFunc(GL_LEQUAL);

 glEnable(GL_BLEND);
 glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

// Settings
//  glFrontFace(GL_CCW);



// UpdateViewport();



 auto& mat_model_ = context.mat_model_;

 mat_model_ = ndk_helper::Mat4::Translation(0, 0, 0);

 ndk_helper::Mat4 mat = ndk_helper::Mat4::RotationX(0.8*M_PI)
                      * ndk_helper::Mat4::RotationY(    M_PI);

 mat_model_ = mat * mat_model_;






 program_stars.compile("Shaders/stars.vsh", "Shaders/stars.fsh");

 program_stars.bindAttribLocation(GLOBE_ATTRIB_Position,"Position");
 program_stars.bindAttribLocation(GLOBE_ATTRIB_Normal  ,"Normal"  );
 program_stars.bindAttribLocation(GLOBE_ATTRIB_Tex     ,"Tex"     );

 program_stars.link();



 program_earth.compile("Shaders/globe-earth.vsh", "Shaders/globe-earth.fsh");

 program_earth.bindAttribLocation(GLOBE_ATTRIB_Position,"Position");
 program_earth.bindAttribLocation(GLOBE_ATTRIB_Normal  ,"Normal"  );
 program_earth.bindAttribLocation(GLOBE_ATTRIB_Tex     ,"Tex"     );

 program_earth.link();



 program_cloud.compile("Shaders/globe-cloud.vsh", "Shaders/globe-cloud.fsh");

 program_cloud.bindAttribLocation(GLOBE_ATTRIB_Position,"Position");
 program_cloud.bindAttribLocation(GLOBE_ATTRIB_Normal  ,"Normal"  );
 program_cloud.bindAttribLocation(GLOBE_ATTRIB_Tex     ,"Tex"     );

 program_cloud.link();



 const double radius = 30.0, eps = 0.1;

 const int nx = 32, ny = 16;

 geometry_stars .Init(nx,ny,9000.0,GL_STATIC_DRAW);
 geometry_earth .Init(nx,ny,radius,GL_STATIC_DRAW);




 num_cloud_layers = 3;

 geometry_cloud = new GlobeGeometry[num_cloud_layers];

 for(int i=0;i<num_cloud_layers;i++)
  geometry_cloud[i].Init(nx,ny,radius+(i+1)*eps,GL_STATIC_DRAW);



#if 0
 glGenTextures(1,&texture);
 glBindTexture(GL_TEXTURE_2D,texture);

 glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
 glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);

 glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
 glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

 int32_t width = 100, height = 100;

// unsigned char* image = LPGPU2::LoadImage("Images/land_shallow_topo_2048.jpg",width,height);
 unsigned char* image = create_image(width,height);

 glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,width,height,0,GL_RGBA,GL_UNSIGNED_BYTE,image);

 delete[] image;

 glGenerateMipmap(GL_TEXTURE_2D);

// glBindTexture(GL_TEXTURE_2D,0); // unbind
#endif

   texture_stars    .Load("Images/Nasa/Stars/starmap_8k.jpg");
// texture_night    .Load("Images/Nasa/Night/dnb_land_ocean_ice.2012.3600x1800.jpg");
   texture_cloud    .Load("Images/Nasa/Cloud/cloud_combined_2048.jpg");

   texture_month[ 0].Load("Images/Nasa/Months/01-jan/world.topo.bathy.200401.3x5400x2700.png");
// texture_month[ 1].Load("Images/Nasa/Months/02-feb/world.topo.bathy.200402.3x5400x2700.png");
// texture_month[ 2].Load("Images/Nasa/Months/03-mar/world.topo.bathy.200403.3x5400x2700.png");
// texture_month[ 3].Load("Images/Nasa/Months/04-apr/world.topo.bathy.200404.3x5400x2700.png");
// texture_month[ 4].Load("Images/Nasa/Months/05-may/world.topo.bathy.200405.3x5400x2700.png");
// texture_month[ 5].Load("Images/Nasa/Months/06-jun/world.topo.bathy.200406.3x5400x2700.png");
   texture_month[ 6].Load("Images/Nasa/Months/07-jul/world.topo.bathy.200407.3x5400x2700.png");
// texture_month[ 7].Load("Images/Nasa/Months/08-aug/world.topo.bathy.200408.3x5400x2700.png");
// texture_month[ 8].Load("Images/Nasa/Months/09-sep/world.topo.bathy.200409.3x5400x2700.png");
// texture_month[ 9].Load("Images/Nasa/Months/10-oct/world.topo.bathy.200410.3x5400x2700.png");
// texture_month[10].Load("Images/Nasa/Months/11-nov/world.topo.bathy.200411.3x5400x2700.png");
// texture_month[11].Load("Images/Nasa/Months/12-dec/world.topo.bathy.200412.3x5400x2700.png");
}



void Globe::Render(ProfilingContext& context) {//double tim) {
 // Set up Projection and Model-View matrices for the shaders
 double tim = context.getTime();

 auto& mat_projection_ = context.mat_projection_;
 auto& mat_view_       = context.mat_view_;

 ndk_helper::Mat4 mat_vp = mat_projection_ * mat_view_;


 {
  // Bind the buffers
  geometry_stars.bind();

  // Pass the vertex data
  int32_t stride = sizeof(GLOBE_VERTEX);

  VertexAttribGuard guard_attrib_Position(GLOBE_ATTRIB_Position, 3, GL_FLOAT, GL_FALSE, stride, BUFFER_OFFSET(0                  ));
  VertexAttribGuard guard_attrib_Normal  (GLOBE_ATTRIB_Normal  , 3, GL_FLOAT, GL_FALSE, stride, BUFFER_OFFSET(3 * sizeof(GLfloat)));
  VertexAttribGuard guard_attrib_Tex     (GLOBE_ATTRIB_Tex     , 2, GL_FLOAT, GL_FALSE, stride, BUFFER_OFFSET(6 * sizeof(GLfloat)));

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


 {
  // Bind the geometry buffers
  geometry_earth.bind();

  // Pass the vertex data
  int32_t stride = sizeof(GLOBE_VERTEX);

  VertexAttribGuard guard_attrib_Position(GLOBE_ATTRIB_Position, 3, GL_FLOAT, GL_FALSE, stride, BUFFER_OFFSET(0                  ));
  VertexAttribGuard guard_attrib_Normal  (GLOBE_ATTRIB_Normal  , 3, GL_FLOAT, GL_FALSE, stride, BUFFER_OFFSET(3 * sizeof(GLfloat)));
  VertexAttribGuard guard_attrib_Tex     (GLOBE_ATTRIB_Tex     , 2, GL_FLOAT, GL_FALSE, stride, BUFFER_OFFSET(6 * sizeof(GLfloat)));

  // Instruct pipeline to use our shader program
  program_earth.use();

  // Texturing...
  program_earth.surface0 (texture_month[ 0]);
  program_earth.surface1 (texture_month[ 0]);

  // Update uniforms
  glUniformMatrix4fv(program_earth.matrix_projection_, 1, GL_FALSE, mat_vp   .Ptr());
  glUniformMatrix4fv(program_earth.matrix_view_      , 1, GL_FALSE, mat_view_.Ptr());

  glUniform1f       (program_earth.interpolation_    , tim-std::floor(tim));

  // Draw!
  geometry_earth.draw(GL_TRIANGLES, BUFFER_OFFSET(0));

  // Unbind
  geometry_earth.unbind();
 }


 for(int i=0;i<num_cloud_layers;i++) {
  geometry_cloud[i].bind();

  int32_t stride = sizeof(GLOBE_VERTEX);

  VertexAttribGuard guard_attrib_Position(GLOBE_ATTRIB_Position, 3, GL_FLOAT, GL_FALSE, stride, BUFFER_OFFSET(0                  ));
  VertexAttribGuard guard_attrib_Normal  (GLOBE_ATTRIB_Normal  , 3, GL_FLOAT, GL_FALSE, stride, BUFFER_OFFSET(3 * sizeof(GLfloat)));
  VertexAttribGuard guard_attrib_Tex     (GLOBE_ATTRIB_Tex     , 2, GL_FLOAT, GL_FALSE, stride, BUFFER_OFFSET(6 * sizeof(GLfloat)));

  // Instruct pipeline to use our shader program
  program_cloud.use();

  // Texturing...
  program_cloud.surface(texture_cloud);

  // Update uniforms
  glUniformMatrix4fv(program_cloud.matrix_projection_, 1, GL_FALSE, mat_vp   .Ptr());
  glUniformMatrix4fv(program_cloud.matrix_view_      , 1, GL_FALSE, mat_view_.Ptr());

  double u = i*1.0/num_cloud_layers; const double speed = 0.01;

  glUniform1f       (program_cloud.phase_            , tim*speed*(u-0.5)+u);
  glUniform1f       (program_cloud.A_                ,                   u);
  glUniform1f       (program_cloud.B_                ,                 1-u);

  // Draw!
  geometry_cloud[i].draw(GL_TRIANGLES, BUFFER_OFFSET(0));

  // Unbind
  geometry_cloud[i].unbind();
 }
}



GlobeProgram::GlobeProgram() {}

void GlobeProgram::uniforms() {
 matrix_projection_ = getUniformLocation("uPMatrix");
 matrix_view_       = getUniformLocation("uMVMatrix");
}



GlobeProgramStars::GlobeProgramStars():
 surface(*this,0,"surface") // texture unit 0
{}

void GlobeProgramStars::uniforms() {
 GlobeProgram::uniforms();
}



GlobeProgramEarth::GlobeProgramEarth():
 surface0(*this,0,"surface0"), // texture unit  0
 surface1(*this,1,"surface1")  // texture unit  1
{}

void GlobeProgramEarth::uniforms() {
 GlobeProgram::uniforms();

 interpolation_ = getUniformLocation("interpolation");
}



GlobeProgramCloud::GlobeProgramCloud():
 surface(*this,0,"surface") // texture unit 0
{}

void GlobeProgramCloud::uniforms() {
 GlobeProgram::uniforms();

 phase_ = getUniformLocation("phase");
 A_     = getUniformLocation("A");
 B_     = getUniformLocation("B");
}



void GlobeElements::Init(std::size_t nx, std::size_t ny, GLenum mode) {
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



void GlobeVertices::Init(GlobeElements& elem, double radius, GLenum mode) {
  xsiz = elem.xsize();
  ysiz = elem.ysize();

  GLOBE_VERTEX* dat = new GLOBE_VERTEX[xsiz*ysiz];

  for(int i=0,o=0;i<xsiz;i++    ) {float u = i/(xsiz-1.0);
  for(int j=0    ;j<ysiz;j++,o++) {float v = j/(ysiz-1.0);
   coords(u,v,dat[o].nrm);

   dat[o].pos[0] = radius * dat[o].nrm[0];
   dat[o].pos[1] = radius * dat[o].nrm[1];
   dat[o].pos[2] = radius * dat[o].nrm[2];

   dat[o].tex[0] = 1-u;
   dat[o].tex[1] =   v;
  }}

  Vertices::Init(xsiz*ysiz,sizeof(GLOBE_VERTEX),dat,mode);

  delete[] dat;
}



void GlobeGeometry::Init(int nx, int ny, double radius, GLenum mode) {
 elements.Init(nx,ny          ,mode);
 vertices.Init(elements,radius,mode);
}

void GlobeGeometry::bind() {
 elements.bind();
 vertices.bind();
}

void GlobeGeometry::unbind() {
 elements.unbind();
 vertices.unbind();
}
