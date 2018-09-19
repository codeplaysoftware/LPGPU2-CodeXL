/*
 * The MIT License
 *
 * Copyright (c) 2016-17 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */
#ifndef INCLUDE_apps_DrawTexCube_hpp
#define INCLUDE_apps_DrawTexCube_hpp



#include "App.hpp"

#include "cube_data.h"



class DrawTexCube: public DirectDrawApp {
 MVP mvp;

 float phase;

public:
 void update_uniforms(std::size_t it) {
  phase += 0.005f;

  vertUni.mvp   = mvp;

  vertUni.phase = phase;
  fragUni.phase = phase;
 }

 DrawTexCube(Instance& instance, float p = 0)

  :DirectDrawApp(
    instance,

    vertUni,fragUni,

    "Shaders/DrawTexCube/DrawTexCube.vert",
    "Shaders/DrawTexCube/DrawTexCube.frag"
   )

  ,phase(p)

  ,myTex1(*this,2,"lunarg.ppm")                                                        // binding = 2
  ,myTex2(*this,3,"Images/Nasa/Months/01-jan/world.topo.bathy.200401.3x5400x2700.ppm") // binding = 3
 {
  init_vertices(12 * 3, (void*)g_vb_texture_Data, sizeof(g_vb_texture_Data), sizeof(g_vb_texture_Data[0]));

  float fov = glm::radians(45.0f);

  if (info.width > info.height) fov *= getAspect();

  mvp.Proj = glm::perspective(fov, getAspect(), 0.1f, 100.0f);

  mvp.View = glm::lookAt(glm::vec3(-5,  3, -10), // Position
                         glm::vec3( 0,  0,   0), // Focus
                         glm::vec3( 0, -1,   0)  // Up
                        );
 }

 TextureData myTex1,myTex2;

 struct {
  glm::mat4 mvp;
  float     phase;
 } vertUni;

 struct {
  float phase;
 } fragUni;
}; // DrawTexCube



#endif
