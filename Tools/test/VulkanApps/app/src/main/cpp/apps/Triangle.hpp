/*
 * The MIT License
 *
 * Copyright (c) 2016-17 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */
#ifndef INCLUDE_apps_Triangle_hpp
#define INCLUDE_apps_Triangle_hpp



#include "App.hpp"



// Two triangles tat span the screen, making it completely white
// Comment out the following line for just one triangle...

#define TWO_TRIANGLES



class Triangle: public IndirectDrawApp {
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

 Triangle(Instance& instance, float p = 0)

  :IndirectDrawApp(
    instance,

    vertUni,fragUni,

    "Shaders/Triangle/Triangle.vert",
    "Shaders/Triangle/Triangle.frag"
   )

  ,vertices(4,{ // width
    -1.0,-1.0,0.0,1.0, // t0,v0
     1.0,-1.0,0.0,1.0, // t0,v1
    -1.0, 1.0,0.0,1.0, // t0,v2
#ifdef TWO_TRIANGLES
    -1.0, 1.0,0.0,1.0, // t1,v0
     1.0,-1.0,0.0,1.0, // t1,v1
     1.0, 1.0,0.0,1.0  // t1,v2
#endif
   })

  ,indices {
    0,1,2,
#ifdef TWO_TRIANGLES
    3,4,5
#endif
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
}; // Triangle



#endif
