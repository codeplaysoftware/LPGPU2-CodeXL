/*
 * The MIT License
 *
 * Copyright (c) 2016-17 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */
#ifndef INCLUDE_apps_Sphere_hpp
#define INCLUDE_apps_Sphere_hpp



#include "App.hpp"



class SphereVertices: public Vertices {

 std::size_t nx,ny; // Sphere discretized as nx*ny grid of rectangles (of 2 triangles each)

 void coords(float u, float v, float r, float* p) {
  float lon = M_PI * (u - 0.5) * 2;         // [-180 .. 180] (but in radians)
  float lat = M_PI * (v - 0.5);             // [ -90 ..  90]

  p[0] = r * std::sin(lon) * std::cos(lat); // spherical polar coords
  p[1] = r *                 std::sin(lat);
  p[2] = r * std::cos(lon) * std::cos(lat);
  p[3] = 1.0f;                              // position vector 'w' component
  p[4] = u;                                 // Texture: U
  p[5] = v;                                 // Texture: V
 }

public:
 SphereVertices(std::size_t x, std::size_t y, float radius):Vertices(6),nx(x),ny(y) {
  resize(6*nx*ny);

  for(int i=0,o=0;i<nx;i++    ) {float u = i/(nx-1.0);
  for(int j=0    ;j<ny;j++,o++) {float v = j/(ny-1.0);
   coords(u,v,radius,&(*this)[6*o]);
  }}
 }
}; // SphereVertices



class SphereIndices: public Indices {
 std::size_t num,nx,ny;

 int index(int i, int n) {while(i<0) i+=n; i%=n; return i;}

 int index(int i, int j, int nx, int ny) {return index(i,nx)*ny+index(j,ny);}

 enum {UPPER_TRI_ONLY = false}; // Handy for debugging - you can see the mesh by dropping out half the tri's

public:
 SphereIndices(std::size_t nx, std::size_t ny):num(6*(nx-1)*(ny-1)),nx(nx),ny(ny) {
  if (UPPER_TRI_ONLY) num/=2;

  resize(num);

  if (UPPER_TRI_ONLY) {
   for(int i=1,o=0;i<nx;i++    )
   for(int j=1    ;j<ny;j++,o++) {
    (*this)[3*o  ] = index(i  ,j  ,nx,ny);
    (*this)[3*o+1] = index(i-1,j  ,nx,ny);
    (*this)[3*o+2] = index(i  ,j-1,nx,ny);
   }
  } else {
   for(int i=1,o=0;i<nx;i++    )
   for(int j=1    ;j<ny;j++,o++) {
    (*this)[6*o  ] = index(i  ,j  ,nx,ny);
    (*this)[6*o+1] = index(i-1,j  ,nx,ny);
    (*this)[6*o+2] = index(i  ,j-1,nx,ny);
    (*this)[6*o+3] = index(i  ,j-1,nx,ny);
    (*this)[6*o+4] = index(i-1,j  ,nx,ny);
    (*this)[6*o+5] = index(i-1,j-1,nx,ny);
   }
  }
 }
}; // SphereIndices



class SphereMVP: public MVP {
 Instance& instance;

public:
 SphereMVP(Instance& instance):instance(instance) {
  auto aspect = instance.getAspect();
  auto fov    = glm::radians(45.0f) * (aspect>1?aspect:1);

  Proj = glm::perspective(fov, aspect, 0.1f, 100.0f);

  View = glm::lookAt(glm::vec3( 0,  5, -10), // Position
                     glm::vec3( 0,  0,   0), // Focus
                     glm::vec3( 0,  1,   0)  // Up
                     );
 }
}; // SphereMVP



class Sphere: public IndirectDrawApp {
 SphereVertices vertices;
 SphereIndices  indices;

public:
 template<typename VU, typename FU>
 Sphere(Instance& instance, VU& vertUni, FU& fragUni, std::size_t n, float radius, int binding, std::string shaderstub, std::vector<std::string> texfiles)

  :IndirectDrawApp(
    instance,

    vertUni,fragUni,

    shaderstub + ".vert",
    shaderstub + ".frag",

    binding
   )

  ,vertices(n,2*n,radius)
  , indices(n,2*n)

  ,textures(texfiles.size())
 {
  for(std::size_t i=0;i<textures.size();i++) textures[i].init(*this,binding+i,texfiles[i].c_str());

  init_vertices(vertices);
  init_indices (indices );

  // This is how to offset the sphere (will needed for the Uber app)
  // centre = glm::vec3(0.0,0.0,5.0);
 }

 std::vector<TextureData> textures;
}; // Sphere



class GlobeSphereMVP: public SphereMVP {
 glm::vec3 centre;  // centre of the globe in the scene

public:
 GlobeSphereMVP(Instance& instance):SphereMVP(instance),phase() {}

 void update() {
  phase += 0.005f;

  Model = glm::translate(glm::mat4(1.0),centre)
        * glm::rotate   (glm::mat4(1.0),phase,glm::vec3(0,1,0));
 }

 void setCentre(glm::vec3 centre) {this->centre = centre;}

 float phase;
}; // GlobeSPhereMVP



class GlobeSphere: public Sphere {
protected:
 GlobeSphereMVP mvp;

 template<typename VU, typename FU>
 GlobeSphere(Instance& instance, VU& vertUni, FU& fragUni, std::size_t n, float radius, float speed, int binding, std::string shaderstub, std::vector<std::string> texfiles)
  :Sphere(instance, vertUni, fragUni, n, radius, binding, shaderstub, texfiles)

  ,mvp(instance)

  ,amp(0.0f)
 {}

public:
 void setCentre(glm::vec3 centre) {mvp.setCentre(centre);}

 float amp; // splurging amplitude

 void setAmp(float amp) {this->amp = amp;}
}; // GlobeSphere



class Earth: public GlobeSphere {
 void update_uniforms(std::size_t it) {
  mvp.update();

  vertUni.mvp    = mvp;
  vertUni.phase  = mvp.phase;
  vertUni.amp    = amp;

  fragUni.phase  = mvp.phase;
 }

public:
 Earth(Instance& instance, std::size_t n, float radius, float speed, int binding)
  :GlobeSphere(
    instance,vertUni,fragUni,n,radius,speed,binding,

    "Shaders/Globe/Earth", // stub for "Earth.vert", "Earth.frag"

    {
     "Images/Nasa/Months/01-jan/world.topo.bathy.200401.3x5400x2700.ppm", // descriptor location = binding
     "Images/Nasa/Months/07-jul/world.topo.bathy.200407.3x5400x2700.ppm"  // descriptor location = binding + 1
    }
   )
 {}

 struct {
  glm::mat4 mvp;
  float     phase;
  float     amp;
 } vertUni;

 struct {
  float phase;
 } fragUni;

 std::size_t bindings() const {return 2;}
}; // Earth



class Cloud: public GlobeSphere {
 void update_uniforms(std::size_t it) {
  mvp.update();

  vertUni.mvp    = mvp;
  vertUni.phase  = mvp.phase;
  vertUni.amp    = amp;

  fragUni.offset = mvp.phase * offset;
  fragUni.A      = A;
  fragUni.B      = B;
 }

public:
 Cloud(Instance& instance, std::size_t n, float radius, float speed, int binding)
  :GlobeSphere(
    instance,vertUni,fragUni,n,radius,speed,binding,

    "Shaders/Globe/Cloud", // stub for "Cloud.vert", "Cloud.frag"

    {"Images/Nasa/Cloud/cloud_combined_2048.ppm"}
   )
  ,A(1.0)
  ,B(0.0)
 {
  offset = speed;
 }

 float A,B;    // Cloud morphing parameters to make cloud layers different (latitudinal asymmetry - see shader)
 float offset; // Cloud movement parameter

 struct {
  glm::mat4 mvp;
  float     phase;
  float     amp;
 } vertUni;

 struct {
  float offset;
  float A;
  float B;
 } fragUni;

 std::size_t bindings() const {return 1;}
}; // Cloud



class Stars: public GlobeSphere {
 void update_uniforms(std::size_t it) {
  mvp.update();

  vertUni.mvp    = mvp;
  vertUni.phase  = mvp.phase;

  fragUni.phase  = mvp.phase;
 }

public:
 Stars(Instance& instance, std::size_t n, float radius, float speed, int binding)
  :GlobeSphere(
    instance,vertUni,fragUni,n,radius,speed,binding,

    "Shaders/Globe/Stars", // Shader filename stub

    {"Images/Nasa/Stars/tycho-skymap.ppm"}
   )
 {}

 struct {
  glm::mat4 mvp;
  float     phase;
 } vertUni;

 struct {
  float phase;
 } fragUni;

 std::size_t bindings() const {return 1;}
}; // Stars



class CloudStack: public App {
 Cloud cloud,cloud2,cloud3,cloud4,cloud5;

public:
 CloudStack(Instance& instance, std::size_t n, float radius, float delta, int binding):App(instance)
  ,cloud (instance, n, radius +     delta,-0.1, binding    )
  ,cloud2(instance, n, radius + 2 * delta, 0.1, binding + 1)
  ,cloud3(instance, n, radius + 3 * delta, 0.2, binding + 2)
  ,cloud4(instance, n, radius + 4 * delta, 0.3, binding + 3)
  ,cloud5(instance, n, radius + 5 * delta, 0.4, binding + 4)
 {}

 void render(std::size_t it) {
  cloud .render(it);
  cloud2.render(it);
  cloud3.render(it);
  cloud4.render(it);
  cloud5.render(it);
 }

 void setCentre(glm::vec3 centre) {
  cloud .setCentre(centre);
  cloud2.setCentre(centre);
  cloud3.setCentre(centre);
  cloud4.setCentre(centre);
  cloud5.setCentre(centre);
 }

 void setAmp(float amp) {
  cloud .setAmp(amp);
  cloud2.setAmp(amp);
  cloud3.setAmp(amp);
  cloud4.setAmp(amp);
  cloud5.setAmp(amp);
 }

 std::size_t bindings() const {return 5;}
}; // CloudStack



// Simple Globe app with earth oscilating between summer and winter under a single cloud layer and starmap
class GlobeBase: public App {
 Earth      earth;
 Stars      stars;
 CloudStack clouds;

public:
 GlobeBase(Instance& instance, float radius, int binding = 2):App(instance)
  ,earth (instance, 40, radius, 1.0  , binding    ) // 2 binding points (summer and winter)
  ,stars (instance, 40,   20.0, 0.125, binding + 2) // 1 binding point
  ,clouds(instance, 40, radius, 0.025, binding + 3) // 5 binding points (cloud layers)
 {}

 void render(std::size_t it) {
  earth .render(it);
  stars .render(it);
  clouds.render(it);
 }

 void setCentre(glm::vec3 centre) {
  earth .setCentre(centre);
  clouds.setCentre(centre);
 }

 void setAmp(float amp) {
  earth .setAmp(amp);
  clouds.setAmp(amp);
 }

 std::size_t bindings() const {
  return earth .bindings()
       + stars .bindings()
       + clouds.bindings();
 }
}; // GlobeBase



class Globe: public GlobeBase {
public:
 Globe(Instance& instance, int binding = 2):GlobeBase(instance,2.0f,binding) {}
}; // Globe



class Uber: public App {
 enum {NUM = 4};

 GlobeBase* globe[NUM];

public:
 Uber(Instance& instance):App(instance) {
  int binding = 2;

  float radius = 2.5f;

  float spacing = radius;

  glm::vec3 v[]={
   {-spacing,-spacing,-spacing},
   {       0, spacing,-spacing},
   { spacing,-spacing,-spacing},
   {       0,       0, spacing}
  };

  for(int i=0;i<NUM;i++) {
   globe[i]=new GlobeBase(instance,radius,binding);

   globe[i]->setCentre(v[i]);
   globe[i]->setAmp   (1.0f); // Splurging!

   binding += globe[i]->bindings();
  }
 }

~Uber() {for(GlobeBase* g:globe) delete g;}

 void render(std::size_t it) {for(auto g:globe) g->render(it);}
}; // Uber



#endif
