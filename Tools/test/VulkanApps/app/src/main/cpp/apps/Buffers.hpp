/*
 * The MIT License
 *
 * Copyright (c) 2016-17 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */
#ifndef INCLUDE_apps_Buffers_hpp
#define INCLUDE_apps_Buffers_hpp



#include <initializer_list>



class Vertices: public std::vector<float> {
public:
 Vertices(size_t n):stride(n) {}

 Vertices(size_t n, std::initializer_list<float> c):std::vector<float>(c),stride(n) {
  assert(!(size()%n));
 }

 size_t stride;
}; // Vertices



class Indices: public std::vector<uint32_t> {
public:
 Indices() = default;

 Indices(std::initializer_list<uint32_t> c):std::vector<uint32_t>(c) {}
}; // Indices



#endif
