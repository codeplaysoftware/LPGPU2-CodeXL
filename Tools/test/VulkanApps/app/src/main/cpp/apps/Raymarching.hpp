/*
 * The MIT License
 *
 * Copyright (c) 2016-17 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */
#ifndef INCLUDE_apps_Raymarching_hpp
#define INCLUDE_apps_Raymarching_hpp



#include "App.hpp"



class Raymarching: public IndirectDrawApp {
 Vertices vertices;
  Indices  indices;

 MVP mvp;

 float phase; // timing variable passed to uniforms

public:
 void update_uniforms(std::size_t it) {
  phase += 0.005f;

  vertUni.mvp   = mvp;
  vertUni.phase = phase;

  fragUni.phase = phase;
  fragUni.xres  = getXRes();
  fragUni.yres  = getYRes();
 }

 Raymarching(Instance& instance, float p = 0)

  :IndirectDrawApp(
    instance,

    vertUni,
    fragUni,

    "Shaders/Raymarching/Raymarching.vert",
    "Shaders/Raymarching/Raymarching.frag"
   )

   // Two triangles to span the screen...
  ,vertices(4,{ // using vec4 for pos, so stride = 4
   -1.0,-1.0, 0.0, 1.0, // v0
    1.0,-1.0, 0.0, 1.0, // v1
   -1.0, 1.0, 0.0, 1.0, // v2
    1.0, 1.0, 0.0, 1.0  // v3
   })

  ,indices{
    0,1,2, // tri 1
    3,2,1  // tri 2
   }

  ,phase(p)
 {
  init_vertices(vertices);
  init_indices ( indices);
 }

 struct {
  glm::mat4 mvp;
  float     phase;
 } vertUni;

 struct {
  float phase;
  int   xres;
  int   yres;
 } fragUni;
}; // Raymarching



#endif
