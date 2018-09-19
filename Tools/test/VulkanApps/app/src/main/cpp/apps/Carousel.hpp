/*
 * The MIT License
 *
 * Copyright (c) 2016-17 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */
#ifndef INCLUDE_apps_Carousel_hpp
#define INCLUDE_apps_Carousel_hpp



#include "DrawTexCube.hpp"
#include "Globe.hpp"



class Carousel: public App {
public:
 Carousel(Instance& c):App(c),
  cube1(c,0.0),
  cube2(c,1.0)
 {}

 DrawTexCube cube1,cube2;

 void render(std::size_t it) {
  cube1.render(it);
  cube2.render(it);
 }
}; // Carousel



#endif
