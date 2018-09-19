/*
 * The MIT License
 *
 * Copyright (c) 2016-17 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */
#ifndef INCLUDE_apps_Menger_hpp
#define INCLUDE_apps_Menger_hpp



#include "App.hpp"



class Menger: public DirectDrawApp {
 Vertices vtx;

 int    mengerGeom;
 int    mengerFrag;

 int    nvtx;
 int    stride;

 double scale;
 double frame;

 void nxtvtx() {nvtx++;}

 void addvtx(float x, float y, float z) {
  vtx.resize(stride*(nvtx+1));

  vtx[stride*nvtx+1-1] = x;
  vtx[stride*nvtx+2-1] = y;
  vtx[stride*nvtx+3-1] = z;
  vtx[stride*nvtx+4-1] = 1;
 }

 void quad(double p1, double p2, double p3,
           double u1, double u2, double u3,
           double v1, double v2, double v3, double su, double sv)
 {
  addvtx(p1            ,p2            ,p3            ); nxtvtx();
  addvtx(p1+su*u1      ,p2+su*u2      ,p3+su*u3      ); nxtvtx();
  addvtx(p1      +sv*v1,p2      +sv*v2,p3      +sv*v3); nxtvtx();

  addvtx(p1      +sv*v1,p2      +sv*v2,p3      +sv*v3); nxtvtx();
  addvtx(p1+su*u1      ,p2+su*u2      ,p3+su*u3      ); nxtvtx();
  addvtx(p1+su*u1+sv*v1,p2+su*u2+sv*v2,p3+su*u3+sv*v3); nxtvtx();
 }

 void cube(double p1, double p2, double p3, double s) {
  p1-=0.5*s; p2-=0.5*s; p3-=0.5*s;

  quad(p1,p2,p3, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0,s,s);
  quad(p1,p2,p3, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0,s,s);
  quad(p1,p2,p3, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0,s,s);

  p1+=1.0*s; p2+=1.0*s; p3+=1.0*s;

  quad(p1,p2,p3,-1.0, 0.0, 0.0, 0.0,-1.0, 0.0,s,s);
  quad(p1,p2,p3, 0.0,-1.0, 0.0, 0.0, 0.0,-1.0,s,s);
  quad(p1,p2,p3, 0.0, 0.0,-1.0,-1.0, 0.0, 0.0,s,s);
 }

 void mengersheets(double p1, double p2, double p3,
                   double u1, double u2, double u3,
                   double v1, double v2, double v3,
                   double w1, double w2, double w3, int n)
 {
  if (!n)
   quad(p1,p2,p3,u1,u2,u3,v1,v2,v3,1.0,1.0);
  else
   for(int i=0;i<=2;i++)
    mengersheets(p1+w1*i/3.0,p2+w2*i/3.0,p3+w3*i/3.0,u1,u2,u3,v1,v2,v3,w1/3.0,w2/3.0,w3/3.0,n-1);
 }

 void menger(double s, int n) {
  double p1=-s/2,p2=-s/2,p3=-s/2;

  double u1=  s,u2=0.0,u3=0.0;
  double v1=0.0,v2=  s,v3=0.0;
  double w1=0.0,w2=0.0,w3=  s;

  mengersheets(p1,p2,p3,u1,u2,u3,v1,v2,v3,w1,w2,w3,n); // Generate a stack of sheets recursively in Z direction
  mengersheets(p1,p2,p3,v1,v2,v3,w1,w2,w3,u1,u2,u3,n); // Generate a stack of sheets recursively in Y direction
  mengersheets(p1,p2,p3,w1,w2,w3,u1,u2,u3,v1,v2,v3,n); // Generate a stack of sheets recursively in X direction

  quad(p1+s,p2  ,p3  ,         v1,v2,v3,w1,w2,w3,1.0,1.0);
  quad(p1  ,p2+s,p3  ,u1,u2,u3,         w1,w2,w3,1.0,1.0);
  quad(p1  ,p2  ,p3+s,u1,u2,u3,v1,v2,v3,         1.0,1.0);
 }

 enum {STRIDE = 4}; // x,y,z,w

public:
 void update_uniforms(std::size_t it) {
  frame = frame + 1.0;

  vertUni.frame = frame;

  vertUni.xres  = getXRes();
  vertUni.yres  = getYRes();

  fragUni.level = mengerFrag;
 }

 Menger(Instance& instance, float p = 0)

  :DirectDrawApp(
    instance,

    vertUni,fragUni,

    "Shaders/Menger/Menger.vert",
    "Shaders/Menger/Menger.frag"
   )

  ,vtx(STRIDE)
 {
  // The 'correct' value of mengerGeom is mengerFrag+1, however, you might not be able
  // to afford that much geometry, because the number of sheets involve is 3 * (1 + 3 ^ mengerGeom)
  mengerGeom = 2;

  // Menger recursion level for the holes in the sponge
  mengerFrag = 3;

  // Vertex count
  nvtx = 0;

  // Number of float elements per vertex
  stride = 4;

  // Scaling parameter controling the size of the cube
  scale = 1.0;

  menger(scale,mengerGeom);

  init_vertices(vtx);
 }

 struct {
  float frame;
  int   xres;
  int   yres;
 } vertUni;

 struct {
  int   level;
 } fragUni;
}; // Menger



#endif
