/*
 * The MIT License
 *
 * Copyright (c) 2016-17 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */
class CarouselBase: public AppRenderer {
 std::vector<AppRenderer*> apps;

 bool init_every_scene;

 double scenetime;

 int prev_scene = -1;

 enum {FPSCAP = 100};

public:
 CarouselBase(bool ies = false, double st = 5):init_every_scene(ies),scenetime(st) {}

 void add(AppRenderer& app) {apps.push_back(&app);}

 void InitEverySceneOn () {init_every_scene = true ;}
 void InitEverySceneOff() {init_every_scene = false;}

 void ReplayInit() {
  if (!init_every_scene) for(auto& app:apps) app->ReplayInit();
 }

 void ReplayRender(double tim) {
  static auto then = throttle.now();

  int scene = int(tim/scenetime)%apps.size();

  RenderScene(tim,scene);

  auto now = throttle.now();

  auto fps = now == then ? 0.0 : 1.0 / (now - then);

  if (fps < FPSCAP) lpgpu2_LogUserCounter(LPGPU2_USER_COUNTER_0,(std::uint32_t)fps);

  then = now;

  prev_scene=scene;
 }

 void RenderScene(double tim, int scene) {
  if (scene != prev_scene && init_every_scene) apps[scene]->ReplayInit();

  apps[scene]->ReplayRender(tim);
 }
}; // CarouselBase



class Carousel: public CarouselBase {
 Raymarching raymarching;
 Overdraw    overdraw;
 Menger      menger;
 Hypercube   hypercube;
 OverdrawTex overdrawtex;

public:
 Carousel(bool ies = false, double s = 5):CarouselBase(ies,s) {
  add(raymarching);
  add(overdraw   );
  add(menger     );
  add(hypercube  );
  add(overdrawtex);

// both uncommented means use default behaviour...
  InitEverySceneOff();
//InitEverySceneOn ();
 }
}; // Carousel
