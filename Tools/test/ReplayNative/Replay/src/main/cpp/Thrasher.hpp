#ifndef INCLUDE_Thrasher_hpp
#define INCLUDE_Thrasher_hpp
/*
 * The MIT License
 *
 * Copyright (c) 2016-17 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */



#include "lpgpu2_api.h"



#include <vector>
#include <thread>



class Thrasher {

 size_t count; // number of outstanding work packets

 std::mutex mtx;

 std::vector<std::thread*> threads;

 bool decrement() {
  std::lock_guard<std::mutex> lk(mtx);

  bool r=!!count;

  if (r) count--;

  lpgpu2_LogAffinityCounter();

  return r;
 }

 void sleep() {std::this_thread::sleep_for(std::chrono::milliseconds(1));}

 void thrash() {for(;;) if (decrement()) task(); else sleep();}

 virtual void task() {sleep();} // The actual work

public:
 Thrasher(size_t n, uint mask):count(0),threads(n) {
  for(auto& thread:threads) thread = new std::thread([this,mask]{
   lpgpu2_SetThreadAffinityMask(mask);

   thrash();
  });
 }

 void work(size_t n=1) { // add 'n' work packets to each thread
  if (!size()) return;

  std::lock_guard<std::mutex> lk(mtx);

  count += n*size();
 }

 void wait() {while(count) sleep();}

 size_t size() const {return threads.size();}
}; // Thrasher



#endif
