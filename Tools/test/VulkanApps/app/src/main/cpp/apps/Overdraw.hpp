/*
 * The MIT License
 *
 * Copyright (c) 2016-17 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */
#ifndef INCLUDE_apps_Overdraw_hpp
#define INCLUDE_apps_Overdraw_hpp



#include "App.hpp"



class Overdraw: public DirectDrawApp {
 Vertices vtx;

 float phase;

 int num;
 int ntri;
 double opacity;
 int order;
 int from,to,step;

 template <typename T> int sgn(T val) {return (T(0) < val) - (val < T(0));}

 enum {STRIDE = 4}; // x,y,z,1

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
 }

public:
 void update_uniforms(std::size_t it) {
  phase += 0.005f;

  vertUni.phase   = phase;

  fragUni.phase   = phase;
  fragUni.opacity = opacity;
  fragUni.xres    = getXRes();
  fragUni.yres    = getYRes();
 }

 Overdraw(Instance& instance, float p = 0)

  :DirectDrawApp(
    instance,

    vertUni,fragUni,

    "Shaders/Overdraw/Overdraw.vert",
    "Shaders/Overdraw/Overdraw.frag"
   )

  ,vtx(STRIDE)

  ,phase(p)
 {
  // Number of sheets in z
  num = 50;

  // Opacity of the sheets
  opacity = 0.05;

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

 struct {
  float phase;
 } vertUni;

 struct {
  float phase;
  float opacity;
  int   xres;
  int   yres;
 } fragUni;
}; // Overdraw



#endif
