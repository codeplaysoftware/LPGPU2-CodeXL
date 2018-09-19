/*
 * The MIT License
 *
 * Copyright (c) 2016-17 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */
#ifndef INCLUDE_apps_OverdrawTex_hpp
#define INCLUDE_apps_OverdrawTex_hpp



#include "App.hpp"



class OverdrawTex: public DirectDrawApp {
 Vertices vtx;

 MVP mvp;

 float phase;

 int num;
 int ntri;
 double opacity;
 int order;
 int from,to,step;

 template <typename T> int sgn(T val) {return (T(0) < val) - (val < T(0));}

 enum {STRIDE = 6}; // x,y,z,1,u,v

 // Create points for the quad in array, vtx
 void quad(int i, int t) {
  const int s = STRIDE;

  vtx.resize(s*3*(t+2));

  int p=3*t;

  coords(&vtx[s*p+1-1],i,-1, 1); p++;
  coords(&vtx[s*p+1-1],i,-1,-1); p++;
  coords(&vtx[s*p+1-1],i, 1,-1); p++;

  coords(&vtx[s*p+1-1],i,-1, 1); p++;
  coords(&vtx[s*p+1-1],i, 1,-1); p++;
  coords(&vtx[s*p+1-1],i, 1, 1); p++;
 }

 void coords(float* p, int i, double U, double V) {
  double t = -(i-1.0)/(num-1);

  p[0] = U + 0.2 * t * sgn(U);
  p[1] = V + 0.2 * t * sgn(V);
  p[2] = t;
  p[3] = 1;

  p[4] = (U+1)/2;
  p[5] = (V+1)/2;
 }

public:
 void update_uniforms(std::size_t it) {
  phase += 0.005f;

  vertUni.mvp     = mvp;

  vertUni.phase   = phase;

  fragUni.phase   = phase;

  fragUni.opacity = opacity;
 }

 OverdrawTex(Instance& instance, float p = 0)

  :DirectDrawApp(
    instance,

    vertUni,fragUni,

    "Shaders/OverdrawTex/OverdrawTex.vert",
    "Shaders/OverdrawTex/OverdrawTex.frag"
   )

  ,vtx(STRIDE)

  ,phase(p)

  // binding = 2 because vertex and fragment uniform blocks are at binding = 0 and 1 respectively
  ,texture(*this,2,"Images/Nasa/Months/01-jan/world.topo.bathy.200401.3x5400x2700.ppm")
 {
  // Number of sheets in z
  num = 5;

  // Opacity of the sheets
  opacity = 0.2;

  // Draw order of the sheets
  order = 1; // back to front (= -1 is reversed)

  // Triangle count
  ntri = 0;

  // Set up the draw order for populating the vtx array based on the 'order' variable
  if (order > 0) {from=1;   to=num; step= 1;}
  else           {from=num; to=1;   step=-1;}

  // With everything in place, populating the vertex array is trivial
  for(int i=from;i<=to;i+=step) {
   quad(i,ntri); ntri = ntri + 2;
  }

  init_vertices(vtx);
 }

 TextureData texture;

 struct {
  glm::mat4 mvp;
  float     phase;
 } vertUni;

 struct {
  float phase;
  float opacity;
 } fragUni;
}; // OverdrawTex



#endif
