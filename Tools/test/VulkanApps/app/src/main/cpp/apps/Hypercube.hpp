/*
 * The MIT License
 *
 * Copyright (c) 2016-17 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */
#ifndef INCLUDE_apps_Hypercube_hpp
#define INCLUDE_apps_Hypercube_hpp



#include "App.hpp"



class Hypercube: public DirectDrawApp {
 Vertices vtx;

 MVP mvp;

 float phase;

 float* gfx_vtx() {return vtx.data();}

 void gfx_rotate(double& v1, double& v2, double& v3, double& v4, double a, int i, int j) {
  double s = std::sin(a);
  double c = std::cos(a);

  double v[] {v1,v2,v3,v4};

  double x = v[j-1]*s+v[i-1]*c;
  double y = v[j-1]*c-v[i-1]*s;

  v[i-1]=x;
  v[j-1]=y;

  v1=v[1-1];
  v2=v[2-1];
  v3=v[3-1];
  v4=v[4-1];
 }

 int gfx_dim  () {return 4;}
 int gfx_count() {return vtx.size()/gfx_dim();}

 void gfx_set(int i, double v1, double v2, double v3, double v4) {
  if (vtx.size()<gfx_dim()*i) vtx.resize(gfx_dim()*i);

  auto n=(i-1)*gfx_dim();

  vtx[n+1-1]=v1;
  vtx[n+2-1]=v2;
  vtx[n+3-1]=v3;
  vtx[n+4-1]=v4;
 }

 void gfx_get(int i, double& v1, double& v2, double& v3, double& v4) {
  int d = gfx_dim();

  v1 = vtx[d*(i-1)+1-1];
  v2 = vtx[d*(i-1)+2-1];
  v3 = vtx[d*(i-1)+3-1];
  v4 = vtx[d*(i-1)+4-1];
 }

 void gfx_new(double v1, double v2, double v3, double v4) {
  int n = vtx.size();

  vtx.resize(n+gfx_dim());

  vtx[n+1-1]=v1;
  vtx[n+2-1]=v2;
  vtx[n+3-1]=v3;
  vtx[n+4-1]=v4;
 }

 double delta;
 double scale;

 void quad(
  double p1, double p2, double p3, double p4,
  double u1, double u2, double u3, double u4,
  double v1, double v2, double v3, double v4, double su, double sv)
 {
  double t1,t2,t3,t4;

  t1=p1            ;
  t2=p2            ;
  t3=p3            ;
  t4=p4            ; gfx_new(t1,t2,t3,t4);

  t1=p1+su*u1      ;
  t2=p2+su*u2      ;
  t3=p3+su*u3      ;
  t4=p4+su*u4      ; gfx_new(t1,t2,t3,t4);

  t4=p4      +sv*v4;
  t3=p3      +sv*v3;
  t2=p2      +sv*v2;
  t1=p1      +sv*v1; gfx_new(t1,t2,t3,t4);


  t1=p1      +sv*v1;
  t2=p2      +sv*v2;
  t3=p3      +sv*v3;
  t4=p4      +sv*v4; gfx_new(t1,t2,t3,t4);

  t1=p1+su*u1      ;
  t2=p2+su*u2      ;
  t3=p3+su*u3      ;
  t4=p4+su*u4      ; gfx_new(t1,t2,t3,t4);

  t1=p1+su*u1+sv*v1;
  t2=p2+su*u2+sv*v2;
  t3=p3+su*u3+sv*v3;
  t4=p4+su*u4+sv*v4; gfx_new(t1,t2,t3,t4);
 }



 void cube(
  double p1, double p2, double p3, double p4,
  double u1, double u2, double u3, double u4,
  double v1, double v2, double v3, double v4,
  double w1, double w2, double w3, double w4, double su, double sv, double sw)
 {
  double q1,q2,q3,q4;

  q1=p1;
  q2=p2;
  q3=p3;
  q4=p4;

  quad(q1,q2,q3,q4,u1,u2,u3,u4,v1,v2,v3,v4            , su, sv    );
  quad(q1,q2,q3,q4,u1,u2,u3,u4            ,w1,w2,w3,w4, su    , sw);
  quad(q1,q2,q3,q4            ,v1,v2,v3,v4,w1,w2,w3,w4    , sv, sw);

  q1+=su*u1+sv*v1+sw*w1;
  q2+=su*u2+sv*v2+sw*w2;
  q3+=su*u3+sv*v3+sw*w3;
  q4+=su*u4+sv*v4+sw*w4;

  quad(q1,q2,q3,q4,u1,u2,u3,u4,v1,v2,v3,v4            ,-su,-sv    );
  quad(q1,q2,q3,q4,u1,u2,u3,u4            ,w1,w2,w3,w4,-su    ,-sw);
  quad(q1,q2,q3,q4            ,v1,v2,v3,v4,w1,w2,w3,w4    ,-sv,-sw);
}


 void hypercube(
  double p1, double p2, double p3, double p4,
  double u1, double u2, double u3, double u4,
  double v1, double v2, double v3, double v4,
  double w1, double w2, double w3, double w4,
  double t1, double t2, double t3, double t4, double su, double sv, double sw, double st)
 {
  double q1,q2,q3,q4;

  q1=p1-su*u1/2-sv*v1/2-sw*w1/2-st*t1/2;
  q2=p2-su*u2/2-sv*v2/2-sw*w2/2-st*t2/2;
  q3=p3-su*u3/2-sv*v3/2-sw*w3/2-st*t3/2;
  q4=p4-su*u4/2-sv*v4/2-sw*w4/2-st*t4/2;

  cube(q1,q2,q3,q4,u1,u2,u4,u4,v1,v2,v3,v4,w1,w2,w3,w4            , su, sv, sw    );
  cube(q1,q2,q3,q4,u1,u2,u3,u4,v1,v2,v3,v4            ,t1,t2,t3,t4, su, sv    , st);
  cube(q1,q2,q3,q4,u1,u2,u3,u4            ,w1,w2,w3,w4,t1,t2,t3,t4, su    , sw, st);
  cube(q1,q2,q3,q4            ,v1,v2,v3,v4,w1,w2,w3,w4,t1,t2,t3,t4    , sv, sw, st);

  q1=p1+su*u1/2+sv*v1/2+sw*w1/2+st*t1/2;
  q2=p2+su*u2/2+sv*v2/2+sw*w2/2+st*t2/2;
  q3=p3+su*u3/2+sv*v3/2+sw*w3/2+st*t3/2;
  q4=p4+su*u4/2+sv*v4/2+sw*w4/2+st*t4/2;

  cube(q1,q2,q3,q4,u1,u2,u3,u4,v1,v2,v3,v4,w1,w2,w3,w4            ,-su,-sv,-sw    );
  cube(q1,q2,q3,q4,u1,u2,u3,u4,v1,v2,v3,v4            ,t1,t2,t3,t4,-su,-sv    ,-st);
  cube(q1,q2,q3,q4,u1,u2,u3,u4            ,w1,w2,w3,w4,t1,t2,t3,t4,-su    ,-sw,-st);
  cube(q1,q2,q3,q4            ,v1,v2,v3,v4,w1,w2,w3,w4,t1,t2,t3,t4    ,-sv,-sw,-st);
 }

 enum {STRIDE = 4}; // x,y,z,w

public:
 void update_uniforms(std::size_t it) {
  phase += 1./256;

  vertUni.mvp   = mvp;
  vertUni.phase = phase;

  vertUni.xres  = getXRes();
  vertUni.yres  = getYRes();

  fragUni.xres  = getXRes();
  fragUni.yres  = getYRes();
 }

 Hypercube(Instance& instance, float p = 0)

  :DirectDrawApp(
    instance,

    vertUni,fragUni,

    "Shaders/Hypercube/Hypercube.vert",
    "Shaders/Hypercube/Hypercube.frag"
   )

  ,vtx(STRIDE)

  ,phase(p)
 {
  scale = 1.4;

  double offset[] {0.0,0.0,0.0,0.0};
  double origin[] {0.0,0.0,0.0,0.0};

  double u_unit[] {1.0,0.0,0.0,0.0};
  double v_unit[] {0.0,1.0,0.0,0.0};
  double w_unit[] {0.0,0.0,1.0,0.0};
  double t_unit[] {0.0,0.0,0.0,1.0};

   hypercube (
    origin[0],origin[1],origin[2],origin[3],
    u_unit[0],u_unit[1],u_unit[2],u_unit[3],
    v_unit[0],v_unit[1],v_unit[2],v_unit[3],
    w_unit[0],w_unit[1],w_unit[2],w_unit[3],
    t_unit[0],t_unit[1],t_unit[2],t_unit[3],scale,scale,scale,scale);

  init_vertices(vtx);
 }

 TextureData texture;

 struct {
  glm::mat4 mvp;
  float     phase;
  int       xres;
  int       yres;
 } vertUni;

 struct {
  int xres;
  int yres;
 } fragUni;
}; // Hypercube



#endif
