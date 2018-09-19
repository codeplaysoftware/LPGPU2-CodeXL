/*
 * The MIT License
 *
 * Copyright (c) 2016-17 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */
#ifndef INCLUDE_apps_Correctness_hpp
#define INCLUDE_apps_Correctness_hpp



#include "App.hpp"



// The Correctness app is for state tracking.
class Correctness: public IndirectDrawApp {
 Vertices vertices;
  Indices  indices;

 MVP mvp;

public:
 void update_uniforms(std::size_t it) {
  phase += 0.005f;

  vertUni.mvp   = mvp;

  vertUni.phase = phase;
  fragUni.phase = phase;
 }

 Correctness(Instance& instance, float p = 0)

  :IndirectDrawApp(
    instance,

    vertUni,fragUni,

    "Shaders/Correctness/Correctness.vert",
    "Shaders/Correctness/Correctness.frag"
   )

  ,vertices(4,{ // = width in elements:
    -1.0,-1.0, 0.0, 1.0,
     1.0,-1.0, 0.0, 1.0,
    -1.0, 1.0, 0.0, 1.0,
    -1.0, 1.0, 0.0, 1.0,
     1.0,-1.0, 0.0, 1.0,
     1.0, 1.0, 0.0, 1.0 
   })

  ,indices {
    0,1,2,
    3,4,5
   }

  ,phase(p)

 {
  init_vertices(vertices);
  init_indices (indices );
 }

 float phase;

 struct {
  glm::mat4 mvp;
  float     phase;
 } vertUni;

 struct {
  float phase;
 } fragUni;

 bool escape() const {return true;}
}; // Correctness



#endif
